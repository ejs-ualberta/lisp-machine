#include <stdint.h>
#include <efi.h>
#include <efilib.h>

#include "config.h"


word max(sword x, sword y){
  return x < y ? y : x;
}


word min(sword x, sword y){
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
  word output;
  asm volatile ("movq %1, %%rax;"
		"cmpxchg %2, (%3);"
		"movq %%rax, %0;"
		:"=r"(output)
		:"r"(cmp), "r"(new), "r"(ptr)
		:"%rax", "memory");
 return output;
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
  return (val >> n) | (word)-1 << (sizeof(word)*8 - n);
}


void outb(uint16_t port, uint8_t val){
    asm volatile ( "outb %0, %1" ::"a"(val),"Nd"(port));
}


EFI_STATUS get_char(EFI_INPUT_KEY * input_key){
  EFI_STATUS Status = ST->ConIn->ReadKeyStroke(ST->ConIn, input_key);
  return Status;
}


/* EFI_STATUS get_mouse_pos(){ */
/*   EFI_STATUS Status; */
/*   return Status; */
/* } */
