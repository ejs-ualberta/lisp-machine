#pragma once

#include "stdint.h"


#define MAX_HRES 1024
#define MAX_VRES 768


typedef uintmax_t word;
typedef uint16_t wchar_t;

//extern word * global_heap_start;

// alloc.c
word * init_heap(word * heap_start, word heap_sz);
word * alloc(word * heap, word mem_sz);
void free(word * heap, word * addr);
word * realloc(word * heap, word * addr, word mem_sz);
word get_mem_sz(word * addr);

// asm.c
word * compile(word * code, word code_sz);
void run(word bytecode, word bc_sz);

//datastructures.c
word * array(word * heap, word size, word item_sz);
word * array_append(word * heap, word * arr, word * item);
void array_del(word * heap, word * arr);
