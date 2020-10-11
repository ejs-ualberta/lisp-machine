#include "config.h"

word * syscall_get_char(word * args){
  return (word*)get_char((EFI_INPUT_KEY*)args);
}

word * syscall_print_char(word * args){
  Object * arguments = (Object*)args;
  //return (word*) fb_print_char(arguments->data[0], arguments->data[1], arguments->data[2], objects->data[3]);
  fb_print_uint(fb_start, 0xcafebabe, 0);
  return (word*)0;
}
