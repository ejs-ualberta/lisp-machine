#include <stdint.h>
#include <efi.h>
#include <efilib.h>

#include "config.h"


//TODO: When creating the str instruction, make an argument for number of bits to store, or create some other method of storing only bytes.
//Maybe read from addr, put byte in appropriate place, then atomic cmpxchng back.
//Todo: Add bit to status register to indicate native code execution mode or virtual asm mode, used when jumping?
//TODO: Add bit to the status register used when storing, word mode or byte mode.

const word word_max = UINTMAX_MAX;

word * global_heap_start;


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
  if (!conv_mem_start || !conv_mem_sz){
    ST->ConOut->OutputString(ST->ConOut, L"No memory????\r\n");
    return Status;
  }


  //Status = ST->BootServices->ExitBootServices(ImageHandle, map_key);
  if (EFI_ERROR(Status)){
    ST->ConOut->OutputString(ST->ConOut, L"Could not exit boot services.\r\n");
    return Status;
  }


  global_heap_start = init_heap(conv_mem_start, conv_mem_sz / sizeof(word));


  /* Load and compile kernel */
  word * kernel_src = array(global_heap_start, 64, 1);
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

  word * bytecode = compile(global_heap_start, kernel_src, array_len(kernel_src));

  //TEST CODE BEGIN
  print_uint((word)bytecode, 16, 8);nl(2);
  if (bytecode){
    for (int i = 0; i < array_len(bytecode); ++i){
      print_uint(bytecode[i], 2, 64);nl(1);
    }
  }
  // TEST CODE END

  
  /* Build os here */
  while (1){};

  Disk->Close(Kernel);
  Disk->Close(Disk);
  shutdown();
  
  return Status;
}
