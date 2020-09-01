#pragma once

#include "stdint.h"


#define MAX_HRES 1366
#define MAX_VRES 768


typedef uintmax_t word;
typedef intmax_t sword;
typedef uint16_t wchar_t;


//extern word * global_heap_start;


// debug.c
int uintn_to_str(wchar_t * buf, word buf_sz, word num, word base);
void print_uint(word val, word base, word padding);
void spc(word n);
void nl(word n);


// alloc.c
extern const word hds_sz;
word * init_heap(word * heap_start, word heap_sz);
word * alloc(word * heap, word mem_sz);
void free(word * heap, word * addr);
word * realloc(word * heap, word * addr, word mem_sz);
word get_mem_sz(word * addr);


// asm.c
word * compile(word * heap, word * code, word code_sz);
void run(word * bytecode);


//datastructures.c
typedef struct object{
  word max_sz; //(in umds, includes size of obj, must be here)
  word refcount;
  word type;
  word size;
  word contents[];
}Object;

typedef struct Array_DS{
  word mem_sz;
  word used_sz;
  word item_sz;
}Array;

typedef struct avl_node{
  word prev;
  word left;
  word right;
  word data;
} AVL_Node;
extern const word avl_node_sz;

typedef struct queue{
  word first;
  word last;
}Queue;

typedef struct link{
  word data;
  word next;
  word prev;
}Link;

word * object(word * heap, word * type, word size, word * contents, word n_words);
word * array(word * heap, word size, word item_sz);
word * array_find(word * arr, word * start, word * item, word (*eq_fn)(word *, word *, word *), word * extra_params);
word * array_append(word * heap, word * arr, word * item);
void array_delete(word * heap, word * arr);
word array_capacity(word * arr);
word array_len(word * arr);
word avl_tree_height(word * nd);
void avl_merge(word ** tr, word * addr, word size);
word * avl_find(word ** tr, word data, word (*cmp)(word*, word*));
word * avl_min_ge(word * tree, word data);
word _avl_insert(word ** tr, word * nd, word data, word (*cmp)(word*, word*));
word * _avl_delete(word ** tr, word * tree, word (*cmp)(word*, word*));
word avl_insert(word * heap, word ** tr, word data, word (*cmp)(word*, word*));
word avl_delete(word * heap, word ** tr, word data, word (*cmp)(word*, word*));
word avl_basic_cmp(word * n1, word * n2);
word avl_mem_cmp(word * n1, word * n2);
void print_avl(word * tree, word space, word inc);
word * pair(word * heap, word * obj1, word * obj2);
word pair_strcmp(word * pair1, word * pair2);
word * set(word * heap);
word set_add(word * heap, word * s, word * data, word (*cmp)(word*, word*));
word set_add_str_key(word * heap, word * s, word * key, word * val);
word * queue(word * heap);
word * queue_push(word * heap, word * queue, word data);
word queue_pop(word * heap, word * queue);


//util.c
word max(sword x, sword y);
word min(sword x, sword y);
word strlen(const word * str);
void * memcpy(void * dest, const void * src, size_t n);
void * memset(void * str, int c, size_t n);
word memcmp(word * m1, word * m2, word len);
word atomic_cas(word * ptr, word cmp, word new);
word nat_pow(word base, word exp);
word abs(word x);
word arithmetic_shift_right(word val, word n);
