#include <stdint.h>
#include <efi.h>
#include <efilib.h>

#include "config.h"

extern int int_routine(void);


//TODO: When creating the str instruction, make an argument for number of bits to store, or create some other method of storing only bytes.
//Maybe read from addr, put byte in appropriate place, then atomic cmpxchng back.
//Todo: Add bit to status register to indicate native code execution mode or virtual asm mode, used when jumping?
//TODO: Add bit to the status register used when storing, word mode or byte mode.

const word word_max = UINTMAX_MAX;

word * global_heap_start;
word global_heap_size;

uint32_t * fb_start = 0;
UINTN b_hres = 0;    
UINTN b_vres = 0;


typedef __attribute__ ((__packed__)) struct IDT_Entry {
  uint16_t offset_1;
  uint16_t selector;
  uint8_t ist;
  uint8_t type_attr;
  uint16_t offset_2;
  uint32_t offset_3;
  uint32_t zero;
}idt_entry;


void interrupt_handler(void){
  for (int i = 0; i < 1000; ++i){fb_start[i] = 0x00FFFFFF;}
  outb(0x20, 0x20);
  outb(0xA0, 0x20); // If isr >= 8
}


void patch_idt_entry(idt_entry * IDT, word idx, word handler_addr){
  IDT[idx].offset_1 = handler_addr & 0xffff;
  //IDT[idx].selector = 0x08;
  //IDT[idx].ist = 0;
  //IDT[idx].type_attr = 0x8E;
  IDT[idx].offset_2 = (handler_addr >> 16) & 0xffff;
  IDT[idx].offset_3 = (handler_addr >> 32);
  //IDT[idx].zero = 0;
}


void sidt(word * loc){
  asm volatile("sidt %0"::"m"(*loc):"memory");
}


void shutdown(void){
  ST->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE * SystemTable){
  EFI_STATUS Status;
  EFI_INPUT_KEY Key;

  /* Store the system table in a global variable */
  ST = SystemTable;

  ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

  word * rsdp = 0;
  //TODO: better way to do this?
  EFI_GUID acpi_20_guid = {0x8868e871,0xe4f1,0x11d3, {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}};
  EFI_CONFIGURATION_TABLE * conf_tbl = ST->ConfigurationTable;
  for (word i = 0; i < ST->NumberOfTableEntries; ++i){
    if (!memcmp((word*)&(conf_tbl->VendorGuid), (word*)&acpi_20_guid, sizeof(EFI_GUID)/sizeof(word))){
      rsdp = conf_tbl->VendorTable;
      break;
    }
    ++conf_tbl;
  }
  if (!rsdp){
    ST->ConOut->OutputString(ST->ConOut, L"Cannot find rsdp.\r\n");
    return Status;
  }
  // Check the checksum of the rsdpd.
  uint32_t rsdpd_len = ((uint32_t*)rsdp)[5];
  word rsdpd_chk = 0;
  for (word i = 0; i < rsdpd_len; ++i){
    rsdpd_chk += ((uint8_t*)rsdp)[i];
  }
  if (!rsdpd_chk || (uint8_t)rsdpd_chk){
    ST->ConOut->OutputString(ST->ConOut, L"Checksum for rsdp is invalid.");
    return Status;
  }

  word * xsdt = (word*)(rsdp[3]);
  // Check the checksum of the xsdt.
  uint32_t xsdt_len = ((uint32_t*)xsdt)[1];
  word xsdt_chk = 0;
  for (word i = 0; i < xsdt_len; ++i){
    xsdt_chk += ((uint8_t*)xsdt)[i];
  }
  if (!xsdt_chk || (uint8_t)rsdpd_chk){
    ST->ConOut->OutputString(ST->ConOut, L"Checksum for xsdt is invalid.");
    return Status;
  }
  
  
  /* Note: this whole section needs updating if/when booting on real hardware someday. */
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

  /* R = 0x00FF0000; */
  /* G = 0x0000FF00; */
  /* B = 0x000000FF; */
  fb_start = (uint32_t*)gop->Mode->FrameBufferBase;

  
  /* Get memory map and find the largest chunk of memory available*/
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
  if (!conv_mem_start || conv_mem_sz <= hds_sz * sizeof(word)){
    ST->ConOut->OutputString(ST->ConOut, L"No memory????\r\n");
    return Status;
  }


  global_heap_size = (word)(conv_mem_sz) / sizeof(word) - hds_sz;
  global_heap_start = init_heap(conv_mem_start, conv_mem_sz / sizeof(word));

  
  /* Open kernel code.*/
  EFI_FILE * Kernel;
  EFI_FILE * Disk;
  EFI_LOADED_IMAGE_PROTOCOL * LoadedImage;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * FileSystem;
  ST->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);
  ST->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

  FileSystem->OpenVolume(FileSystem, &Disk);
  Status = Disk->Open(Disk, &Kernel, L"kernel", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
  if (EFI_ERROR(Status)) {
    ST->ConOut->OutputString(ST->ConOut, L"No kernel???");
    return Status;
  }
  EFI_FILE_INFO k_info;
  word k_info_sz = 0;
  // What the heck??? I shouldn't have to do this. I should be able to use sizeof(k_info)
  while (1){
    Status = Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &k_info_sz, (void**)&k_info);
    if (Status != EFI_BUFFER_TOO_SMALL){
      break;
    }
    ++k_info_sz;
  }

  if (EFI_ERROR(Status)) {
    ST->ConOut->OutputString(ST->ConOut, L"Could not get size of kernel.");
    return Status;
  }

  //Load and compile kernel
  word * kernel_src = array(global_heap_start, k_info.FileSize, 1);
  wchar_t contents[2];
  word character = 0;
  contents[1] = '\0';
  word one = 1;
  for (word i = 0; i < k_info.FileSize; ++i){
    Status = Kernel->Read(Kernel, &one, contents);
    if (EFI_ERROR(Status)){
      ST->ConOut->OutputString(ST->ConOut, L"Could not load kernel.\r\n");
      return Status;
    }
    character = (word)*contents;
    kernel_src = array_append(global_heap_start, kernel_src, &character);
    if (!kernel_src){
      ST->ConOut->OutputString(ST->ConOut, L"Out of memory, kernel too big.\r\n");
      return Status;
    }
  }

  /* Status = ST->BootServices->ExitBootServices(ImageHandle, map_key); */
  /* if (EFI_ERROR(Status)){ */
  /*   ST->ConOut->OutputString(ST->ConOut, L"Could not exit boot services.\r\n"); */
  /*   return Status; */
  /* } */

  /* while(1){ */
  /*   EFI_INPUT_KEY kp; */
  /*   EFI_STATUS S = get_char(&kp); */
  /*   if (!EFI_ERROR(S)){ */
  /*     word x = kp.UnicodeChar; */
  /*     x = (x >= 65) ? x - 87 : x - 48; */
  /*     fb_print_char(fb_start, x, 0x00000000, 0x00FFFFFF); */
  /*   } */
  /* } */


  // Doesn't work
  /* EFI_GUID pp_guid = {0x31878c87,0xb75,0x11d5, {0x9a,0x4f,0x00,0x90,0x27,0x3f,0xc1,0x4d}}; */
  /* EFI_SIMPLE_POINTER_PROTOCOL mouse; */
  /* EFI_SIMPLE_POINTER_STATE m_state; */
  /* Status = ST->BootServices->LocateProtocol(&pp_guid, NULL, (void**)&mouse); */
  /* if (EFI_ERROR(Status)){return Status;} */
  /* while (1){ */
  /*   Status = mouse.GetState(&mouse, &m_state); */
  /*   *(fb_start + m_state.RelativeMovementY * b_hres + m_state.RelativeMovementX) = 0x00FFFFFF; */
  /* } */


  word idt = 0;
  sidt(&idt);
  idt >>= 16;
  patch_idt_entry((idt_entry*)idt, 0x80, (word)int_routine);

  
  init_types();
  word * machine_info = init_machine(ImageHandle, SystemTable);
  word * bytecode = compile(global_heap_start, kernel_src, array_len(kernel_src));
  array_delete(global_heap_start, kernel_src);

  //Add stack space to bytecode
  if (!bytecode){fb_print_uint(fb_start, 0xdeadc0de, 0);return 1;}
  word zero = 0;
  for (word i = 0; i < 1024; ++i){
    bytecode = array_append(global_heap_start, bytecode, &zero);
    if (!bytecode){fb_print_uint(fb_start, 0xdeadc0de, 0);return 1;}
  }

  fb_print_uint(fb_start, run(bytecode, machine_info), 0);
  array_delete(global_heap_start, bytecode);

  while (1){
    //fb_print_uint(fb_start, 0xdeadbeef, 0);
  }

  Disk->Close(Kernel);
  Disk->Close(Disk);
  shutdown();
  
  return Status;
}
