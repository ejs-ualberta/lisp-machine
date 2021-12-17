#include "config.h"

word * global_heap_start;
word global_heap_size;
word * exception_fifo;
//TODO: memory locks for alloc/free, etc

/* word kernel_code[] = { */
/* 0x1D7800000000000, */
/* 0x1C68000000001B3, */
/* 0x1CE000000000000, */
/* 0x1DF800000000002, */
/* 0xEFE800000000199, */
/* 0x89FFFE0000000000, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x1C6000000000002, */
/* 0xDDE000000000000, */
/* 0xDEE3FFFFFFFFFFF, */
/* 0xCEE3FFFFFFFFFFC, */
/* 0x1EF400000000030, */
/* 0x1DF800000000002, */
/* 0xEFE80000000013A, */
/* 0xCEE3FFFFFFFFFFF, */
/* 0xCDE000000000000, */
/* 0x1C63FFFFFFFFFFE, */
/* 0xFFFFFFFFFFFFFFF, */
/* 0x1C6000000000003, */
/* 0xDCE3FFFFFFFFFFE, */
/* 0x1CE3FFFFFFFFFFE, */
/* 0xD0E400000000001, */
/* 0xD16400000000002, */
/* 0x10E800000000006, */
/* 0xB087FFFFFFFFFFD, */
/* 0x8910840000000000, */
/* 0x11080000000000F, */
/* 0x8A10840000000000, */
/* 0x8708440000000000, */
/* 0x71740000000000F, */
/* 0x8808440000000000, */
/* 0x8910840000000000, */
/* 0x110800007E01710, */
/* 0xCE8800000000003, */
/* 0x7EF400080000000, */
/* 0xEEFBFFFFFFFFFFE, */
/* 0xCE8800000000004, */
/* 0xBEF400000000020, */
/* 0xBEF7FFFFFFFFFE0, */
/* 0x88E87A0000000000, */
/* 0xDE8800000000004, */
/* 0xCE8800000000003, */
/* 0x7EF400040000000, */
/* 0xEEFBFFFFFFFFFFE, */
/* 0xCE8800000000000, */
/* 0x7EF4000FFFFFFFF, */
/* 0x89E87A0000000000, */
/* 0xEEFBFFFFFFFFFFA, */
/* 0xCEE800000000006, */
/* 0xBEF400000000020, */
/* 0x9EF400080000000, */
/* 0xEEF800000000003, */
/* 0x1EF400000000001, */
/* 0xEFF800000000002, */
/* 0x89EF7A0000000000, */
/* 0xC16400000000002, */
/* 0xC0E400000000001, */
/* 0xCCE400000000000, */
/* 0x2C6000000000003, */
/* 0xEFEC00000000000, */
/* 0x1C6000000000004, */
/* 0xDCE3FFFFFFFFFFD, */
/* 0x1CE3FFFFFFFFFFD, */
/* 0xD0E400000000001, */
/* 0xD16400000000002, */
/* 0xD1E400000000003, */
/* 0x8918C60000000000, */
/* 0x10E800000000006, */
/* 0x110C0000000008C, */
/* 0xD10400000000000, */
/* 0x110C00000000008, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000048003, */
/* 0xD10400000000001, */
/* 0xC17400000000000, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000000008, */
/* 0xD10400000000002, */
/* 0x110C00000048004, */
/* 0xB10BFFFFFFFFFE0, */
/* 0xC1F400000000001, */
/* 0x8810860000000000, */
/* 0xD10400000000003, */
/* 0x8918C60000000000, */
/* 0x110C00000000008, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000000008, */
/* 0xD10400000000004, */
/* 0xC17400000000000, */
/* 0xC1F400000000001, */
/* 0xB18FFFFFFFFFFE0, */
/* 0x8810860000000000, */
/* 0xD10400000000005, */
/* 0x8918C60000000000, */
/* 0x110C00000000008, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000048009, */
/* 0xD10400000000006, */
/* 0x110C00000000008, */
/* 0xD10400000000007, */
/* 0x110C00000048005, */
/* 0xB10BFFFFFFFFFE0, */
/* 0xD10400000000008, */
/* 0x110C00000000004, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000000004, */
/* 0xD10400000000009, */
/* 0x110C00000048006, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000000020, */
/* 0xD1040000000000A, */
/* 0x110C00000000004, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000000004, */
/* 0xD1040000000000B, */
/* 0x110C00000040001, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000000001, */
/* 0xD1040000000000C, */
/* 0x110C00000000008, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000000008, */
/* 0xD1040000000000D, */
/* 0x110C00000001000, */
/* 0xD1040000000000E, */
/* 0x110C00000000004, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x110800000040008, */
/* 0xD1040000000000F, */
/* 0x110C00000000004, */
/* 0xD10400000000010, */
/* 0xD18400000000011, */
/* 0x117400000000000, */
/* 0x89EF7A0000000000, */
/* 0x1EF400000000008, */
/* 0x1C6000000000001, */
/* 0xDDE000000000000, */
/* 0x1DF800000000002, */
/* 0xEFE800000000031, */
/* 0xCDE000000000000, */
/* 0x1C63FFFFFFFFFFF, */
/* 0xEEF800000000002, */
/* 0xEFE8000000000CE, */
/* 0xC1840000000000A, */
/* 0xB18FFFFFFFFFFE0, */
/* 0xB18C00000000020, */
/* 0x918C00000000020, */
/* 0xE1E8000000000CE, */
/* 0xC1840000000000E, */
/* 0xB18FFFFFFFFFFE0, */
/* 0xB18C00000000020, */
/* 0xE1F800000000002, */
/* 0xEFE8000000000CE, */
/* 0x718C0003FFFFFFF, */
/* 0x1E8800000000000, */
/* 0xC17400000000006, */
/* 0xD18800000000000, */
/* 0xC17400000000002, */
/* 0xC18400000000002, */
/* 0xB18C00000000020, */
/* 0xD18800000000000, */
/* 0xC17400000000003, */
/* 0xC18400000000003, */
/* 0xB18C00000000020, */
/* 0xB18FFFFFFFFFFE0, */
/* 0xD18800000000000, */
/* 0xC17400000000004, */
/* 0xC18400000000010, */
/* 0xB18C00000000020, */
/* 0xD18800000000000, */
/* 0xC17400000000005, */
/* 0xC1840000000000C, */
/* 0xB18FFFFFFFFFFE0, */
/* 0xB18C00000000020, */
/* 0xD18800000000000, */
/* 0xC1E400000000003, */
/* 0xC16400000000002, */
/* 0xC0E400000000001, */
/* 0xCCE400000000000, */
/* 0x2C6000000000004, */
/* 0xEFEC00000000000, */
/* 0x1C6000000000003, */
/* 0xDCE3FFFFFFFFFFE, */
/* 0x1CE3FFFFFFFFFFE, */
/* 0xD0E400000000001, */
/* 0xD16400000000002, */
/* 0x89EF7A0000000000, */
/* 0x1EF400007E40206, */
/* 0xC0F400000000000, */
/* 0xB08400000000020, */
/* 0xB087FFFFFFFFFE0, */
/* 0xD0F400000000000, */
/* 0x1EE800000000006, */
/* 0x8910840000000000, */
/* 0x108800000000024, */
/* 0xD0F400000000000, */
/* 0x10880000000000C, */
/* 0xB087FFFFFFFFFE0, */
/* 0x108400000038002, */
/* 0xD0F400000000001, */
/* 0x108800000000002, */
/* 0xB087FFFFFFFFFE0, */
/* 0x108400000000008, */
/* 0xD0F400000000002, */
/* 0x1088000003D0900, */
/* 0xD0F400000000003, */
/* 0xD17400000000004, */
/* 0x89EF7A0000000000, */
/* 0x1EF400000000008, */
/* 0x1C6000000000001, */
/* 0xDDE000000000000, */
/* 0x1DF800000000002, */
/* 0xEFE800000000031, */
/* 0xCDE000000000000, */
/* 0x1C63FFFFFFFFFFF, */
/* 0x1E8800007E40000, */
/* 0xC0F400000000000, */
/* 0x11080000003F000, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x8A10840000000000, */
/* 0x8708440000000000, */
/* 0x8910840000000000, */
/* 0x110800000024000, */
/* 0xB10BFFFFFFFFFE0, */
/* 0x8808440000000000, */
/* 0xD0F400000000000, */
/* 0x8908420000000000, */
/* 0x1E8400007E40012, */
/* 0xC0F400000000000, */
/* 0xB087FFFFFFFFFE0, */
/* 0xB08400000000020, */
/* 0xD0F400000000000, */
/* 0x89EF7A0000000000, */
/* 0x1EF4000000000A0, */
/* 0x1EF7FFFFFFFFFFF, */
/* 0xEEFBFFFFFFFFFFF, */
/* 0x10F400007E40013, */
/* 0xCE8400000000000, */
/* 0xBEF400000000020, */
/* 0xBEF7FFFFFFFFFE0, */
/* 0x1EF40000000C000, */
/* 0xDE8400000000000, */
/* 0x89EF7A0000000000, */
/* 0x1EF4000000000A0, */
/* 0x1EF7FFFFFFFFFFF, */
/* 0xEEFBFFFFFFFFFFF, */
/* 0xCE8400000000000, */
/* 0xBEF400000000020, */
/* 0xBEF7FFFFFFFFFE0, */
/* 0xDE8400000000000, */
/* 0x89EF7A0000000000, */
/* 0x1EF400007E40208, */
/* 0xC0F400000000000, */
/* 0xB087FFFFFFFFFE0, */
/* 0xB08400000000020, */
/* 0x10847FF00000000, */
/* 0xD0F400000000000, */
/* 0x89EF7A0000000000, */
/* 0x1EF400007E40204, */
/* 0xC0F400000000000, */
/* 0xB087FFFFFFFFFE0, */
/* 0xB08400000000020, */
/* 0x108400200000000, */
/* 0xD0F400000000000, */
/* 0x89EF7A0000000000, */
/* 0x8908420000000000, */
/* 0x1EF400007E40205, */
/* 0x108400000000070, */
/* 0xB087FFFFFFFFFE0, */
/* 0x10840000000000B, */
/* 0xD0F400000000000, */
/* 0x89EF7A0000000000, */
/* 0x1EF400007E40206, */
/* 0xC0F400000000000, */
/* 0xB08400000000020, */
/* 0xB087FFFFFFFFFE0, */
/* 0x108400000000301, */
/* 0xD0F400000000000, */
/* 0xC16400000000002, */
/* 0xC0E400000000001, */
/* 0xCCE400000000000, */
/* 0x2C6000000000003, */
/* 0xEFEC00000000000, */
/* 0x1C6000000000003, */
/* 0xDCE3FFFFFFFFFFE, */
/* 0x1CE3FFFFFFFFFFE, */
/* 0xD0E400000000001, */
/* 0xD16400000000002, */
/* 0x8908420000000000, */
/* 0x108400007E40203, */
/* 0xC10400000000000, */
/* 0x710800000000020, */
/* 0xE17BFFFFFFFFFFE, */
/* 0xDE87FFFFFFFFFFD, */
/* 0xC16400000000002, */
/* 0xC0E400000000001, */
/* 0xCCE400000000000, */
/* 0x2C6000000000003, */
/* 0xEFEC00000000000, */
/* 0x1C6000000000003, */
/* 0xDCE3FFFFFFFFFFE, */
/* 0x1CE3FFFFFFFFFFE, */
/* 0xD0E400000000001, */
/* 0xD16400000000002, */
/* 0x8908420000000000, */
/* 0x108400007E40203, */
/* 0xC10400000000000, */
/* 0x710800000000010, */
/* 0xE17BFFFFFFFFFFE, */
/* 0xCE87FFFFFFFFFFD, */
/* 0x7EF4000FFFFFFFF, */
/* 0x91740000000000D, */
/* 0xE17800000000002, */
/* 0x2EF400000000003, */
/* 0xC16400000000002, */
/* 0xC0E400000000001, */
/* 0xCCE400000000000, */
/* 0x2C6000000000003, */
/* 0xEFEC00000000000, */
/* 0x1C6000000000005, */
/* 0xDCE3FFFFFFFFFFC, */
/* 0x1CE3FFFFFFFFFFC, */
/* 0xD0E400000000001, */
/* 0xD16400000000002, */
/* 0xD1E400000000003, */
/* 0xDDE400000000004, */
/* 0x10F400000000000, */
/* 0x8910840000000000, */
/* 0x8CE8440000000000, */
/* 0x7EF40000000FFFF, */
/* 0x91F40000000000A, */
/* 0xE1F800000000002, */
/* 0x1EF400000000003, */
/* 0x11F400000000000, */
/* 0x1DF800000000002, */
/* 0xEFE80000000013A, */
/* 0x110800000000001, */
/* 0xE1FBFFFFFFFFFF7, */
/* 0xCDE400000000004, */
/* 0xC1E400000000003, */
/* 0xC16400000000002, */
/* 0xC0E400000000001, */
/* 0xCCE400000000000, */
/* 0x2C6000000000005, */
/* 0xEFEC00000000000, */
/* 0xEEF800000000002, */
/* 0xEFEC00000000000, */
/* 0xCEF400000000000, */
/* 0xBEF400000000002, */
/* 0xBEF7FFFFFFFFFFE, */
/* 0xEFEC00000000000, */
/* 0xEEF800000000003, */
/* 0x1EF400000000003, */
/* 0xEFEC00000000000, */
/* 0xCEF400000000000, */
/* 0x7EF400000000003, */
/* 0xEFEC00000000000, */
/* 0x1C6000000000002, */
/* 0xDCE3FFFFFFFFFFF, */
/* 0x1CE3FFFFFFFFFFF, */
/* 0xD0E400000000001, */
/* 0xC0F400000000000, */
/* 0xCEF400000000001, */
/* 0x7EF400000000003, */
/* 0xB08400000000002, */
/* 0xB087FFFFFFFFFFE, */
/* 0x88E87A0000000000, */
/* 0xC0E400000000001, */
/* 0xCCE400000000000, */
/* 0x2C6000000000002, */
/* 0xEFEC00000000000, */
/* 0x400, */
/* 0x300, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x0, */
/* 0x1E6800000000026, */
/* 0x1C6000000000001, */
/* 0xDDE000000000000, */
/* 0x1DF800000000002, */
/* 0xEFE8000000000D4, */
/* 0xCDE000000000000, */
/* 0x1C63FFFFFFFFFFF, */
/* 0x1EE800000000194, */
/* 0xDEF400000000000, */
/* 0x1EE800000000195, */
/* 0xDEF400000000000, */
/* 0x1EE800000000196, */
/* 0xDEF400000000000, */
/* 0x1EE800000000197, */
/* 0xDEF400000000000, */
/* 0x1EE800000000198, */
/* 0xDEF400000000000, */
/* 0x1EE800000000192, */
/* 0x1C6000000000001, */
/* 0xDDE000000000000, */
/* 0x1DF800000000002, */
/* 0xEFE80000000005B, */
/* 0xCDE000000000000, */
/* 0x1C63FFFFFFFFFFF, */
/* 0xCEE800000000198, */
/* 0xEFEC00000000000, */
/* 0x0, */
/* }; */

char * kc_ascii = "|>[#T ~>[[v1] [~>[[v2] [:v1]]]]]|>[#F ~>[[v1] [~>[[v2] [:v2]]]]]|>[nil ~>[[x] [:#T]]]|>[id ~>[[x] [:x]]]|>[cons ~>[[v1 v2] [~>[[cond][:>[:>[:cond :v1] :v2]]]]]]|>[car ~>[[lst] [:>[:lst :#T]]]]|>[cdr ~>[[lst] [:>[:lst :#F]]]]|>[sel ~>[[cond v1 v2] [:>[:>[:cons :v1 :v2] :cond]]]]|>[if ~>[[cond v1 v2] [::>[:sel :cond v1 v2]]]]|>[empty ~>[[l] [:>[:l ~>[[head] [~>[[tail] [:#F]]]]]]]]|>[concat ~>[[l1 l2][:>:>[:if :>[:empty |>[x :>[:cdr :l1]]][:id :>[:cons :>[:car :l1] :l2]][:cons :>[:car :l1] :>[:concat :x :l2]]]]]] |>[plist ~>[[l][>:>[:if :>[:empty :l] [] [:>[:car :l] :>[:plist :>[:cdr :l]]]]]]] |>[l1 :>[:cons 1 :nil]] |>[plist ~>[[l][>:>[:if :>[:empty :l] [] [:>[:car :l] :>[:plist :>[:cdr :l]]]]]]] :>[:plist :>[:concat :l1 :nil]]";

void main(word * DeviceTree){
  //TODO: properly get screen size
  //TODO: properly get available memory
  uart_print_uint(DeviceTree, 16);uart_puts("\n");
  word * conv_mem_start = (word*)&_end - ((word)&_end & 7) + 8;
  word conv_mem_sz = 1024 * 1024 * 16;
  global_heap_size = (word)(conv_mem_sz) / sizeof(word) - hds_sz;
  global_heap_start = init_heap(conv_mem_start, conv_mem_sz / sizeof(word));

  word * kernel_code = array(global_heap_start, sizeof(kc_ascii), 1);
  kernel_code = array_append_str(global_heap_start, kernel_code, (char*)kc_ascii);
  extern word * run_prog(word * heap, word * machine, word * code, word code_sz);
  extern word * run_prog_stack(word * heap, word * machine, word * code, word code_sz);
  Object * obj = (Object*)run_prog_stack(global_heap_start, 0, kernel_code, array_len(kernel_code));
  array_delete(global_heap_start, kernel_code);
  rec_obj_print(obj);
  ++((Object*)obj)->refcount;
  object_delete(global_heap_start, obj);uart_puts("\n");
  gc_collect(global_heap_start);
  uart_print_uint(((hds*)global_heap_start)->gc_num_alloced, 16);uart_puts("\n");
  uart_print_uint(((hds*)global_heap_start)->true_num_alloced, 16);uart_puts("\n");
  word check_gc(word * gc_set);
  check_gc(((hds*)global_heap_start)->gc_set);
  breakp();

  /* exception_fifo = concurrent_fifo(global_heap_start, 16, 16, 1); */
  /* word * fb_start = (word*)run(exception_fifo, kernel_code); */
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
