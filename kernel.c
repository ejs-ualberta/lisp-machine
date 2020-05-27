#include <stdint.h>
#include <efi.h>
#include <efilib.h>
#include "config.h"


//TODO: When creating the str instruction, make an argument for number of bits to store, or create some other method of storing only bytes.
//Maybe read from addr, put byte in appropriate place, then atomic cmpxchng back.


typedef uint16_t wchar_t;

int uintn_to_str(wchar_t * buf, UINTN buf_sz, UINTN num, UINTN base){
  if (buf_sz < 2){return -1;}
  if (num == 0){
    buf[0] = '0';
    buf[1] = '\0';
    return 1;
  }

  UINTN i = 0;
  for (; num; ++i){
    if (i >= buf_sz - 1){return -1;}
    UINTN digit = num % base;
    digit += 48;
    if (digit >= 58){
      digit += 7;
      if (digit >= 91){
	digit += 6;
      }
    }
    buf[i] = digit;
    num /= base;
  }

  buf[i--] = '\0';
  for (int j = 0; j < i; ++j, --i){
    wchar_t tmp = buf[j];
    buf[j] = buf[i];
    buf[i] = tmp;
  }

  return i+1;
}

void print_uint(UINTN val){
  wchar_t buf[32];
  uintn_to_str(buf, 32, val, 16);
  ST->ConOut->OutputString(ST->ConOut, buf);
}

void spc(){
ST->ConOut->OutputString(ST->ConOut, L" ");
}

void nl(){
  ST->ConOut->OutputString(ST->ConOut, L"\r\n");
}


typedef uintmax_t word;
const word word_max = UINTMAX_MAX;
const word word_sz = sizeof(word);


typedef struct used_mem_datastructure{
  word mem_sz;
  word mem[];
} umds;

typedef struct free_mem_datastructure{
  word prev;
  word next;
  word mem_sz;
} fmds;

typedef struct heap_datastructure{
  word heap_end;
  word prev;
  word next;
  word fmds_ptr;
} hds;

const word umds_sz = sizeof(umds)/sizeof(word);
const word fmds_sz = sizeof(fmds)/sizeof(word);
const word hds_sz = sizeof(hds)/sizeof(word);
const word min_alloc_sz = fmds_sz;


word * init_heap(word * heap_start, word heap_sz){
  if ((word)heap_start % sizeof(word)){return 0;}
  if (heap_sz < min_alloc_sz + hds_sz){return 0;}

  hds * heap = (hds*)heap_start;
  heap->heap_end = (word)(heap_start + heap_sz);
  heap->prev = 0;
  heap->next = 0;
  heap->fmds_ptr = (word)(heap_start + hds_sz);

  fmds * mem = (fmds*)(heap_start + hds_sz);
  mem->prev = 0;
  mem->next = 0;
  mem->mem_sz = heap_sz - hds_sz;

  return heap_start;
}


word * alloc(word * heap, word mem_sz){
  mem_sz += umds_sz;
  if (mem_sz < min_alloc_sz){mem_sz = min_alloc_sz;}

  hds * h_info = (hds*)heap;
  fmds * mem_ptr = (fmds*)h_info->fmds_ptr;
  while (mem_ptr && mem_ptr->mem_sz < mem_sz){
    mem_ptr = (fmds*)(mem_ptr->next);
  }
  if (!mem_ptr){return 0;}

  /* word offset = 0; */
  if (mem_ptr->mem_sz < mem_sz + min_alloc_sz){
    mem_sz = mem_ptr->mem_sz;

    fmds * prev = (fmds*)mem_ptr->prev;
    if (prev){
      prev->next = mem_ptr->next;
    }else{
      h_info->fmds_ptr = mem_ptr->next;
    }
    
    fmds * next = (fmds*)mem_ptr->next;
    if (next){
      next->prev = mem_ptr->prev;
    }
  }else{
    /* offset = mem_ptr->mem_sz - mem_sz; */
    /* mem_ptr->mem_sz = offset; */
    word os = mem_ptr->mem_sz - mem_sz;
    fmds * new_ptr = (fmds*)((word*)mem_ptr + mem_sz);
    *new_ptr = *mem_ptr;
    new_ptr->mem_sz = os;
    fmds * pv = (fmds*)new_ptr->prev;
    if (pv){
      pv->next = (word)new_ptr;
    }else{h_info->fmds_ptr = (word)new_ptr;}
  }

  umds * mem = (umds*)(mem_ptr);  /* + offset); */
  mem->mem_sz = mem_sz;
  return (word*)&(mem->mem);
}


void free(word * heap, word * addr){
  hds * h_info = (hds*)heap;
  if (addr < (word*)h_info + hds_sz || addr >= (word*)h_info + h_info->heap_end){
    return;
  }
  fmds * mem_ptr = (fmds*)h_info->fmds_ptr;
  fmds * freed_mem = (fmds*)(addr - umds_sz);
  umds * mem_obj = (umds*)(addr - umds_sz);
  word fm_sz = mem_obj->mem_sz;

  if (!mem_ptr){
    h_info->fmds_ptr = (word)freed_mem;
    freed_mem->prev = 0;
    freed_mem->next = 0;
    freed_mem->mem_sz = fm_sz;
    return;
  }

  fmds * next = mem_ptr;
  mem_ptr = 0;
  while (next && next < freed_mem){
    mem_ptr = next;
    next = (fmds*)mem_ptr->next;
  }

  freed_mem->next = 0;
  freed_mem->prev = 0;
  freed_mem->mem_sz = fm_sz;
  if (next){
    if ((word*)freed_mem + fm_sz == (word*)next){
      fmds * next_next = (fmds*)next->next;
      freed_mem->mem_sz += next->mem_sz;
      freed_mem->next = (word)next_next;
      if (next_next){
	next_next->prev = (word)freed_mem;
      }
    }else{
      freed_mem->next = (word)next;
      next->prev = (word)freed_mem;
    }
  }

  if (mem_ptr){
    if ((word*)mem_ptr + mem_ptr->mem_sz == (word*)freed_mem){
      fmds * next_next = (fmds*)freed_mem->next;
      mem_ptr->mem_sz += freed_mem->mem_sz;
      mem_ptr->next = (word)next_next;
      if (next_next){
	next_next->prev = (word)mem_ptr;
      }
    }else{
      freed_mem->prev = (word)mem_ptr;
      mem_ptr->next = (word)freed_mem;
    }
  }else{
    h_info->fmds_ptr = (word)freed_mem;
  }
}


word * realloc(word * heap, word * addr, word mem_sz){
  word * new_addr = alloc(heap, mem_sz);
  if (!new_addr){return 0;}
  umds * old_obj = (umds*)(addr - umds_sz);
  for (word i = 0; i < old_obj->mem_sz; ++i){
    new_addr[i] = addr[i];
  }
  free(heap, addr);
  return new_addr;
}


word compile(word code, word code_sz){

}


void run(word bytecode, word bc_sz){

}


void shutdown(void){
  ST->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE * SystemTable){
  EFI_STATUS Status;
  EFI_INPUT_KEY Key;

  /* Store the system table in a global variable */
  ST = SystemTable;

  /* Get a handle to the GOP */
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  Status = ST->BootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid,
					    NULL,
					    (void**)&gop);
  if (EFI_ERROR(Status) || gop == NULL) {
    ST->ConOut->OutputString(ST->ConOut, L"Could not get GOP handle.\r\n");
    return Status;
  }

  /* TODO: properly get screen size (for booting on real hardware)*/
  /* EFI_EDID_ACTIVE_PROTOCOL * edid_ap; */
  /* Status = ST->BootServices->LocateProtocol(&gEfiEdidActiveProtocolGuid, NULL, (void **)&edid_ap); */
  /* if (EFI_ERROR(Status) || edid_ap == NULL) { */
  /*   ST->ConOut->OutputString(ST->ConOut, L"Could not get EDID handle.\r\n"); */
  /*   return Status; */
  /* } */

  /* Note: this whole section needs updating if/when booting on real hardware someday. */
  uint32_t * fb_start = 0; 
  UINTN b_hres = 0;    
  UINTN b_vres = 0;
  UINTN screen_mode = 0;
  UINTN px_mode = 0;
  UINTN px_per_line = 0;
  UINTN max_mode = gop->Mode->MaxMode;
  for (UINTN i = 0; i < max_mode; ++i){
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION * info;
    UINTN info_sz;
    
    Status = gop->QueryMode(gop, i, &info_sz, &info);
    if (EFI_ERROR(Status)){
      ST->ConOut->OutputString(ST->ConOut, L"Error querying GOP mode.\r\n");
      return Status;
    }
    
    UINTN hres = (UINTN)info->HorizontalResolution;
    UINTN vres = (UINTN)info->VerticalResolution;
    if (hres > b_hres && hres <= MAX_HRES && vres > b_vres && vres <= MAX_VRES){
      b_hres = hres;
      b_vres = vres;
      screen_mode = i;
      px_mode = info->PixelFormat;
      px_per_line = info->PixelsPerScanLine;
    }
  }

  if (!b_hres || !b_vres){
    ST->ConOut->OutputString(ST->ConOut, L"Error: No available screen mode.\r\n");
    return Status;
  }

  Status = gop->SetMode(gop, screen_mode);
  if (EFI_ERROR(Status)){
    ST->ConOut->OutputString(ST->ConOut, L"Unable to set screen mode\r\n");
    return Status;
  }

  /* Red = 0x00FF0000; */
  /* Green = 0x0000FF00; */
  /* Blue = 0x000000FF; */
  fb_start = (uint32_t*)gop->Mode->FrameBufferBase;

  
  /* Get Memory map and find the largest chunk of memory available*/
  UINTN map_sz = 0, map_key, desc_sz = 0;
  UINT32 desc_vn = 0;
  EFI_MEMORY_DESCRIPTOR * Map;

  /* Note: For some reason GetMemoryMap does not return an accurate value in map_sz? */
  Status = ST->BootServices->GetMemoryMap(&map_sz, Map, &map_key, &desc_sz, &desc_vn);
  do {
    Status = ST->BootServices->AllocatePool(EfiLoaderData, map_sz, (void**)&Map);
    if (EFI_ERROR(Status)){
      Status = ST->ConOut->OutputString(ST->ConOut, L"Cannot allocate buffer for memory map.\r\n");
      return Status;
    }
    Status = ST->BootServices->GetMemoryMap(&map_sz, Map, &map_key, &desc_sz, &desc_vn);
    if (Status == EFI_BUFFER_TOO_SMALL){
      ST->BootServices->FreePool(Map);
      ++map_sz;
    }
  } while(Status != EFI_SUCCESS);

  UINTN * conv_mem_start = 0;
  UINTN conv_mem_sz = 0;
  for (int i = 0; i < map_sz / desc_sz; ++i){
    if (Map[i].Type == EfiConventionalMemory){
      UINTN mem_blk_sz = Map[i].NumberOfPages * 4096;
      if (mem_blk_sz > conv_mem_sz){
	conv_mem_sz = mem_blk_sz;
	conv_mem_start = (UINTN*)Map[i].PhysicalStart;
      }      
    }
  }
  if (!conv_mem_start || !conv_mem_sz){
    ST->ConOut->OutputString(ST->ConOut, L"No memory????\r\n");
    return Status;
  }


  Status = ST->BootServices->ExitBootServices(ImageHandle, map_key);
  if (EFI_ERROR(Status)){
    ST->ConOut->OutputString(ST->ConOut, L"Could not exit boot services.\r\n");
    return Status;
  }


  /* Build os here */
  //init_heap(conv_mem_start, conv_mem_sz / sizeof(word));

  while (1){};
  shutdown();
  
  return Status;
}
