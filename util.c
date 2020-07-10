#include "config.h"

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
