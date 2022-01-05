#include "config.h"
#include "kc_ascii.h"

word * global_heap_start;
word global_heap_size;
word * exception_fifo;
//TODO: memory locks for alloc/free, etc

/* word kcode[] = { */
/* 0x01D7800000000000, 0x01C6800000000015, 0x01CE000000000000, 0x01DF800000000002, 0x0EFE800000000011, 0x89FFFE0000000000, 0x01C6000000000002, 0x0DDE000000000000, 0x0DEE3FFFFFFFFFFF, 0x0CEE3FFFFFFFFFFC, 0x01EF400000000030, 0x01DF800000000002, 0x0EF80000000144A0, 0x0CEE3FFFFFFFFFFF, 0x0CDE000000000000, 0x01C63FFFFFFFFFFE, 0x0FFFFFFFFFFFFFFF, 0x01E6800000000006, 0x0F00000000000001, 0x01E80000000A0BB8, 0x0EFEC00000000000, 0x0000000000000000, 0x0000000000000000,   */
/* }; */



// Pass in args in r1d, return result in r1d, return by jumping to zero (so that
// functions that return using lr are compatible, as lr is set to 0)
// Asm fns can call each other and lisp fns can call asm fns, but asm fns can not call lisp fns (yet)
// although lisp functions can be used as "macros" on uncompiled asm.
// One thing to note is that when returning from an asm fn to a lisp fn a lisp object must be returned.
// The code in kernel.asm is no longer compatible, as asm addresses used to be word
// aligned and now they are byte aligned.


//kc_ascii is now in the kc_ascii file.
uint8_t * kc_ascii = __kernel;

void main(word * DeviceTree){
  //TODO: properly get screen size
  //TODO: properly get available memory
  uart_puts("Device Tree: ");uart_print_uint((word)DeviceTree, 16);uart_puts("\n");
  word * conv_mem_start = (word*)&_end - ((word)&_end & (sizeof(word)-1)) + sizeof(word);
  word conv_mem_sz = 1024 * 1024 * 16;
  global_heap_size = (word)(conv_mem_sz) / sizeof(word) - hds_sz;
  global_heap_start = init_heap(conv_mem_start, conv_mem_sz / sizeof(word));

  exception_fifo = concurrent_fifo(global_heap_start, 16, 16, 1);
  word * kernel_code = array(global_heap_start, 64, 1);
  kernel_code = array_append_str(global_heap_start, kernel_code, kc_ascii);
  extern word * run_prog_stack(word * heap, word * exception_fifo, word * code, word code_sz);
  word * obj = (word*)run_prog_stack(global_heap_start, exception_fifo, kernel_code, array_len(kernel_code));
  array_delete(global_heap_start, kernel_code);
  concurrent_fifo_delete(global_heap_start, exception_fifo);
  rec_obj_print(obj);
  ++((Object*)obj)->refcount;
  object_delete(global_heap_start, obj);uart_puts("\n");
  gc_collect(global_heap_start);
  uart_print_uint(((hds*)global_heap_start)->gc_num_alloced, 16);uart_puts("\n");
  uart_print_uint(((hds*)global_heap_start)->true_num_alloced, 16);uart_puts("\n");
  word check_gc(word * gc_set);
  check_gc(((hds*)global_heap_start)->gc_set);
  uart_puts("Done.\n");
  breakp();

  /* exception_fifo = concurrent_fifo(global_heap_start, 16, 16, 1); */
  /* word * regs = init_regs(global_heap_start, kcode); */
  /* word * fb_start = (word*)run(exception_fifo, regs, 0); */
  /* uart_puts("Framebuffer: ");uart_print_uint(fb_start, 16);uart_puts("\n"); */
  /* free(global_heap_start, regs); */
  /* concurrent_fifo_delete(global_heap_start, exception_fifo); */
  /* //array_delete(global_heap_start, kernel_code); */

  /* word * kernel_code = array(global_heap_start, 512, 1); */
  /* while(1){ */
  /*   word c = uart_getc(); */
  /*   kernel_code = array_append(global_heap_start, kernel_code, &c); */
  /*   uart_send(c); */
  /*   if (!c){ */
  /*     word * kbc = compile(global_heap_start, kernel_code, array_len(kernel_code)); */
  /*     if (!kbc){continue;} */
  /*     uart_puts("\n"); */
  /*     for (word i = 0; i < array_len(kbc); ++i){ */
  /* 	uart_puts("0x"); */
  /* 	uart_print_uint(kbc[i], 16); */
  /* 	uart_puts(",\n"); */
  /*     } */
  /*   } */
  /* } */
}


void exc_handler(uint32_t type, uint32_t esr, uint32_t elr, uint32_t spsr, uint32_t far){
  word s_num = (esr & ((word)-1 >> 40)) | (word)type << 32;
  concurrent_fifo_add(exception_fifo, &s_num);
}
