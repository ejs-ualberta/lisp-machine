#include "config.h"


word max(sword x, sword y){
  return x < y ? y : x;
}


word strlen(const word * str){
  const word * s = str;
  for (; *str; ++str){}
  return (word)(str-s) / sizeof(word);
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


word atomic_cas(word * ptr, word cmp, word new){
  word output;
  asm volatile ("movq %1, %%rax;"
		"cmpxchg %2, (%3);"
		"movq %%rax, %0;"
		:"=r"(output)
		:"r"(cmp), "r"(new), "r"(ptr)
		:"%rax", "memory"
       );
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
