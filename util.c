#include "config.h"

word strlen(const word * str){
  const word * s = str;
  for (; *str; ++str){}
  return (word)(str-s) / sizeof(word);
}
