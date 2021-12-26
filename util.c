#include "config.h"

#define MMIO_BASE       0x3F000000
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))


void uart_send(unsigned int c) {
  while(*UART0_FR & 0x20){asm volatile("nop");}
  *UART0_DR = c;
}


char uart_getc() {
  char ch;
  while(*UART0_FR & 0x10){asm volatile("nop");}
  ch = (char)(*UART0_DR);
  return ch == '\r' ? '\n' : ch;
}


void uart_puts(char * s) {
  while(*s) {
    if(*s=='\n'){
      uart_send('\r');
    }
    uart_send(*s++);
  }
}


void uart_print_uint(word val, word base){
  word buf[65];
  uintn_to_str(buf, 65, val, base);
  for (word i = 0; buf[i] && i < 65; ++i){
    uart_send((uint8_t)buf[i]);
  }
}


void uart_padded_uint(word val, word base, word padding){
  word buf[sizeof(word)*8 + 1];
  word max = sizeof(word)*8 + 1;
  word len = uintn_to_str(buf, max, val, base);
  for (word i = len; i < padding; ++i){
    uart_puts("0");
  }
  for (word i = 0; buf[i] && i < max; ++i){
    uart_send((uint8_t)buf[i]);
  }
}


word uintn_to_str(word * buf, word buf_sz, word num, word base){
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
    uint16_t tmp = buf[j];
    buf[j] = buf[i];
    buf[i] = tmp;
  }

  return retval+1;
}

word max(sword x, sword y){
  return x < y ? y : x;
}


word umax(word x, word y){
  return x < y ? y : x;
}


word min(sword x, sword y){
  return x < y ? x : y;
}


word umin(word x, word y){
  return x < y ? x : y;
}


word strlen(const word * str){
  const word * s = str;
  for (; *str; ++str){}
  return (word)(str-s) / sizeof(word);
}


word strncmp(uint8_t * s1, uint8_t * s2, word len){
  for (word i = 0; i < len; ++i){
    if (s1[i] > s2[i]){
      return (word)-1;
    }else if (s1[i] > s2[i]){
      return (word)1;
    }
  }
  return 0;
}


// Keeps the compiler from dying
void * memcpy(void * dest, const void * src, size_t n){
  word lim = n / sizeof(word);
  word rem = n % sizeof(word);
  for (word i = 0; i < lim; ++i){
    *((word*)dest) = *((word*)src);
  }
  for (word i = 0; i < rem; ++i){
    *((char*)dest) = *((char*)src);
  }
  return dest;
}


// Also keeps the compiler from dying
void * memset(void * str, int c, size_t n){
  for (word i = 0; i < n; ++i){
    *((char*)str + i) = c;
  }
  return str;
}


word memcmp(word * m1, word * m2, word len){
  for (word i = 0; i < len; ++i){
    if (m1[i] > m2[i]){
      return (word)-1;
    }else if (m1[i] < m2[i]){
      return 1;
    }
  }
  return 0;
}


word atomic_cas(word * ptr, word cmp, word new){
  return __sync_val_compare_and_swap(ptr, cmp, new);
}


word nat_pow(word base, word exp){
  word res = 1;
  for (; exp; --exp){
    res *= base;
  }
  return res;
}


word abs(word x){
  return (x >> ((sizeof(word) * 8 - 1)) & 1) ? -x : x;
}


word arithmetic_shift_right(word val, word n){
  if (!(val & 1 << (sizeof(word)-1))){
    return val >> n;
  }
  return (val >> n) | (word)-1 << (sizeof(word)*8 - n);
}


void breakp(){;}
