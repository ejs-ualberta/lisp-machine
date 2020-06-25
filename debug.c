#include <stdint.h>
#include <efi.h>
#include <efilib.h>

#include "config.h"


int uintn_to_str(wchar_t * buf, word buf_sz, word num, word base){
  if (buf_sz < 2){return -1;}
  if (num == 0){
    buf[0] = '0';
    buf[1] = '\0';
    return 1;
  }

  word i = 0;
  for (; num; ++i){
    if (i >= buf_sz - 1){return -1;}
    word digit = num % base;
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

void print_uint(word val, word base){
  wchar_t buf[65];
  uintn_to_str(buf, 65, val, base);
  ST->ConOut->OutputString(ST->ConOut, buf);
}

void spc(word n){
  for (word i = 0; i < n; ++i){
    ST->ConOut->OutputString(ST->ConOut, L" ");
  }
}

void nl(word n){
  for (word i = 0; i < n; ++i){
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");
  }
}
