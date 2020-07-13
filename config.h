#pragma once

#include "stdint.h"


#define MAX_HRES 1024
#define MAX_VRES 768


typedef uintmax_t word;
typedef uint16_t wchar_t;

//extern word * global_heap_start;


// debug.c
int uintn_to_str(wchar_t * buf, word buf_sz, word num, word base);
void print_uint(word val, word base, word padding);
void spc(word n);
void nl(word n);


// alloc.c
word * init_heap(word * heap_start, word heap_sz);
word * alloc(word * heap, word mem_sz);
void free(word * heap, word * addr);
word * realloc(word * heap, word * addr, word mem_sz);
word get_mem_sz(word * addr);


// asm.c
word * compile(word * heap, word * code, word code_sz);
void run(word * bytecode, word bc_sz);


//datastructures.c
word * object(word * heap, word * type, word size, word * contents, word n_words);
void object_delete(word * heap, word * obj);
word * array(word * heap, word size, word item_sz);
word * array_find(word * arr, word * start, word * item, word (*eq_fn)(word *, word *, word *), word * extra_params);
word * array_append(word * heap, word * arr, word * item);
void array_delete(word * heap, word * arr);
word array_capacity(word * arr);
word array_len(word * arr);


//util.c
word strlen(const word * str);
void * memcpy(void * dest, const void * src, size_t n);
void * memset(void * str, int c, size_t n);
word atomic_cas(word * ptr, word cmp, word new);
