#include <stdint.h>
#include <efi.h>
#include <efilib.h>


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
  EFI_STATUS Status;
  EFI_INPUT_KEY Key;

  /* Store the system table in a global variable */
  ST = SystemTable;

  /* Disable watchdog timer */
  /* SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL); */

  /* Clear screen. */
  /* for (int i = 0; i < 24; ++i){ */
  /*   for (int j = 0; j < 80; ++j){ */
  /*     Status = ST->ConOut->OutputString(ST->ConOut, L" "); */
  /*     if (EFI_ERROR(Status)) */
  /* 	return Status; */
  /*   } */
  /*   Status = ST->ConOut->OutputString(ST->ConOut, L"\r\n"); */
  /*   if (EFI_ERROR(Status)) */
  /*     return Status; */
  /* } */

  //Status = ST->ConOut->OutputString(ST->ConOut, L"Hello World\n\r");
  //if (EFI_ERROR(Status))
  //  return Status;

  /* /\* Flush input buffer to ignore any previous keystrokes *\/ */
  /* Status = ST->ConIn->Reset(ST->ConIn, FALSE); */
  /* if (EFI_ERROR(Status)) */
  /*   return Status; */

  /* /\* Wait for key press *\/ */
  /* while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY) ; */

  /* Get a handle to the GOP */
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  Status = ST->BootServices->HandleProtocol( ST->ConsoleOutHandle, 
					     &gEfiGraphicsOutputProtocolGuid, 
					     (void **) &gop);
  if (EFI_ERROR(Status) || gop == NULL) {
    ST->ConOut->OutputString(ST->ConOut, L"Could not get GOP handle.\r\n");
    return Status;
  }

  uint64_t max_mode = 0;
  max_mode = gop->Mode->MaxMode;
  for (uint64_t i = 0; i < max_mode; ++i){
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION * info;
    UINTN info_sz;
    
    Status = gop->QueryMode(gop, i, &info_sz, &info);
    if (EFI_ERROR(Status)){
      ST->ConOut->OutputString(ST->ConOut, L"Error querying GOP mode.\r\n");
      return Status;
    }
  }


  /* Get Memory map */
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

  Status = ST->BootServices->ExitBootServices(ImageHandle, map_key);
  if (EFI_ERROR(Status)){
    ST->ConOut->OutputString(ST->ConOut, L"Could not exit boot services.\r\n");
    return Status;
  }


  /* Build os here */
  while(1){};
  
  return Status;
}
