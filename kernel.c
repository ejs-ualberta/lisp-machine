#include <stdint.h>
#include <efi.h>
#include <efilib.h>
#include "config.h"


/* typedef uint16_t wchar_t; */

/* int uintn_to_str(wchar_t * buf, UINTN buf_sz, UINTN num, UINTN base){ */
/*   if (buf_sz < 2){return -1;} */
/*   if (num == 0){ */
/*     buf[0] = '0'; */
/*     buf[1] = '\0'; */
/*     return 1; */
/*   } */

/*   UINTN i = 0;  */
/*   for (; num; ++i){ */
/*     if (i >= buf_sz - 1){return -1;} */
/*     UINTN digit = num % base; */
/*     digit += 48; */
/*     if (digit >= 58){ */
/*       digit += 7; */
/*       if (digit >= 91){ */
/* 	digit += 6; */
/*       } */
/*     } */
/*     buf[i] = digit; */
/*     num /= base; */
/*   } */

/*   buf[i--] = '\0'; */
/*   for (int j = 0; j < i; ++j){ */
/*     char tmp = buf[i-j]; */
/*     buf[i-j] = buf[j]; */
/*     buf[j] = tmp; */
/*   } */

/*   return i+1; */
/* } */


EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
  EFI_STATUS Status;
  EFI_INPUT_KEY Key;

  /* Store the system table in a global variable */
  ST = SystemTable;

  /* Get a handle to the GOP */
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  Status = ST->BootServices->HandleProtocol( ST->ConsoleOutHandle, 
					     &gEfiGraphicsOutputProtocolGuid, 
					     (void **) &gop);
  if (EFI_ERROR(Status) || gop == NULL) {
    ST->ConOut->OutputString(ST->ConOut, L"Could not get GOP handle.\r\n");
    return Status;
  }

  UINTN * fb_start = 0; 
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

  fb_start = (UINTN*)gop->Mode->FrameBufferBase;
  for (int i = 0; i < 10; ++i){
    for (int j = 0; j < b_hres; ++j){
      *(fb_start + i * px_per_line + j) = 0x00FF0000;
    }
  }
  for (int i = 10; i < 20; ++i){
    for (int j = 0; j < b_hres; ++j){
      *(fb_start + i * px_per_line + j) = 0x0000FF00;
    }
  }
  for (int i = 20; i < 30; ++i){
    for (int j = 0; j < b_hres; ++j){
      *(fb_start + i * px_per_line + j) = 0x000000FF;
    }
  }
  for (int i = 30; i < 40; ++i){
    for (int j = 0; j < b_hres; ++j){
      *(fb_start + i * px_per_line + j) = 0xFFFFFFFF;
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
