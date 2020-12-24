#include "config.h"


const word kerning = 4;

uint16_t chars[36][16] = {
  {0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF},
  {0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180},
  {0xFFFF, 0xFFFF, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0xFFFF, 0xFFFF, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0xFFFF, 0xFFFF},
  {0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003},
  {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003},
  {0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003},
  {0x0180, 0x03C0, 0x0660, 0x0C30, 0x1818, 0x300C, 0x6006, 0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003},
  {0xFFFC, 0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF},
  {0xFF00, 0xFF80, 0xC0C0, 0xC060, 0xC030, 0xC018, 0xC00C, 0xC006, 0xC006, 0xC00C, 0xC018, 0xC030, 0xC060, 0xC0C0, 0xFF80, 0xFF00},
  {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000},
  {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC00F, 0xC00F, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF},
  {0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003},
  {0xFFFF, 0xFFFF, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0xFFFF, 0xFFFF},
  {0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0xFFFF, 0xFFFF},
  {0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003},
  {0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0xC183, 0xC183, 0xC183, 0xC183, 0xC183, 0xC183, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003},
  {0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003},
  {0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000},
  {0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC183, 0xC183, 0xC183, 0xC183, 0xFFFF, 0xFFFF},
  {0xFFFC, 0xFFFC, 0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xC00C, 0xFFFF, 0xFFFF, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003},
  {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xFFFF, 0xFFFF, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0xFFFF, 0xFFFF},
  {0xFFFF, 0xFFFF, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180},
  {0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xFFFF, 0xFFFF},
  {0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0x6006, 0x300C, 0x1818, 0x0C30, 0x0660, 0x03C0, 0x0180},
  {0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC003, 0xC183, 0xC183, 0xC183, 0xC183, 0xC183, 0xFFFF, 0xFFFF},
  {0xC003, 0xC003, 0x6006, 0x6006, 0x300C, 0x300C, 0x1818, 0x1FF8, 0x1FF8, 0x1818, 0x300C, 0x300C, 0x6006, 0x6006, 0xC003, 0xC003},
  {0xC003, 0xC003, 0x6006, 0x6006, 0x300C, 0x300C, 0x1818, 0x1818, 0x0FF0, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180},
  {0xFFFE, 0xFFFE, 0x000C, 0x0018, 0x0030, 0x0060, 0x00C0, 0x0180, 0x0300, 0x0600, 0x0C00, 0x1800, 0x3000, 0x6000, 0xFFFF, 0xFFFF},
};

word char_w = sizeof(chars[0][0]) * 8;
word char_h = sizeof(chars[0]) / sizeof(chars[0][0]);


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
  word retval = i;
  for (int j = 0; j < i; ++j, --i){
    wchar_t tmp = buf[j];
    buf[j] = buf[i];
    buf[i] = tmp;
  }

  return retval+1;
}


void print_uint(word val, word base, word padding){
  wchar_t buf[65];
  word len = uintn_to_str(buf, 65, val, base);
  for (word i = len; i < padding; ++i){
    ST->ConOut->OutputString(ST->ConOut, L"0");
  }
  ST->ConOut->OutputString(ST->ConOut, buf);
}


void print_cstr(char * str){
  for (word i = 0; str[i]; ++i){
    uint16_t chr[2] = {str[i], 0};
    ST->ConOut->OutputString(ST->ConOut, chr);
  }
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


void fb_print_char(uint32_t * loc, word val, uint32_t bc, uint32_t fc){
  uint16_t * chr_ptr = chars[val];

  extern uint32_t * fb_start;
  extern UINTN b_hres;    
  extern UINTN b_vres;

  for (word i = 0; i < char_h; ++i){
    for (word j = 0; j < char_w; ++j){
      uint32_t * ptr = (loc + i * b_hres + j);
      if (chr_ptr[i] & (1 << (char_w - j - 1))){
	*ptr = fc;
      }else{
	*ptr = bc;
      }
    }
  }
}


void fb_print_uint(uint32_t * loc, word val, word padding){
  wchar_t buf[65];
  word len = uintn_to_str(buf, 65, val, 16);
  for (word i = len; i < padding; ++i){
    fb_print_char(loc, 0, 0x00000000, 0x00FFFFFF);
    loc += char_w + kerning;
  }
  for (word i = 0; i < len; ++i){
    word x = buf[i];
    x = (x >= 65) ? x - 55 : x - 48;
    fb_print_char(loc, x, 0x00000000, 0x00FFFFFF);
    loc += char_w + kerning;
  }
}
