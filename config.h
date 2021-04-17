#pragma once

#include <stdint.h>
#include <efi.h>
#include <efilib.h>

#define MAX_HRES 1366
#define MAX_VRES 768


typedef uintmax_t word;
typedef intmax_t sword;
typedef uint16_t wchar_t;


extern word * global_heap_start;
extern word global_heap_size;

extern uint32_t * fb_start;
extern UINTN b_hres;    
extern UINTN b_vres;


// debug.c
int uintn_to_str(wchar_t * buf, word buf_sz, word num, word base);
void print_uint(word val, word base, word padding);
void print_cstr(char * str);
void spc(word n);
void nl(word n);
void fb_print_char(uint32_t * loc, word val, uint32_t bc, uint32_t fc);
void fb_print_uint(uint32_t * loc, word val, word padding);


// alloc.c
extern const word hds_sz;
extern word * gc_set;
word get_mem_sz(word * addr); 
word * init_heap(word * heap_start, word heap_sz);
word * gc_init(word * heap);
word * alloc(word * heap, word mem_sz);
word * gc_alloc(word * heap, word n);
void free(word * heap, word * addr);
void gc_free(word * heap, word * addr);
word * realloc(word * heap, word * addr, word mem_sz);
word * gc_realloc(word * heap, word * addr, word mem_sz);
void gc_collect(word * heap, word * gc_set);

// asm.c
word * compile(word * heap, word * code, word code_sz);
word run(word * bytecode, word * machine);


//datastructures.c
extern word * num_type;
extern word * string_type;
extern word * array_type;
extern word * set_type;
extern word * function_type;
extern word * cell_type;
/* extern word * native_type; */
/* extern word * asm_type; */

extern const word obj_sz;

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
word * object_append_word(word * heap, word * obj, word data);
word * object_append(word * heap, word * obj, word * data);
word * object_expand(word * heap, word * obj, word new_sz);
void obj_print(word * obj);
void rec_obj_print(word * obj);
void object_delete(word * heap, word * obj);
word * obj_array(word * heap, word size);
word obj_array_size(word * arr);
void obj_array_append(word * heap, word * arr, word * data);
word * obj_array_idx(word * arr, word idx);
void set_obj_array_idx(word * arr, word idx, word * val);
void obj_array_delete(word * heap, word * obj);
word * array(word * heap, word size, word item_sz);
word * array_find(word * arr, word * start, word * item, word (*eq_fn)(word *, word *, word *), word * extra_params);
word * array_append(word * heap, word * arr, word * item);
word * array_append_str(word * heap, word * arr, uint8_t * str);
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
void init_types(void);
word * pair(word * heap, word * obj1, word * obj2);
word obj_cmp(word * obj1, word * obj2);
word set_keycmp(word * pair1, word * pair2);
word * set(word * heap);
word set_add(word * heap, word * s, word * data, word (*cmp)(word*, word*));
word * set_remove(word * heap, word * s, word * data, word (*cmp)(word*, word*));
word set_add_str_key(word * heap, word * s, word * key, word * val);
word set_set_key(word * s, word * key, word * val);
word * set_remove_str_key(word * heap, word * s, word * key);
word * in_set(word * set, word * obj);
word * set_get_value(word * set, word * obj);
void print_set(word * s);
void set_delete(word * heap, word * set);
word * queue(word * heap);
word * queue_push(word * heap, word * queue, word data);
word queue_last(word * heap, word * queue);
word queue_pop(word * heap, word * queue);
void num_negate(word * num);
word * num_add(word * heap, word * num1, word * num2);
word * num_shift_left(word * heap, word * num, word shf);
word * num_shift_right(word * heap, word * num, word shf);
word num_le(word * heap, word * num1, word * num2);
word * num_shift(word * heap, word * num1, word * num2);
word * num_and(word * heap, word * num1, word * num2);
word * num_or(word * heap, word * num1, word * num2);
word * num_xor(word * heap, word * num1, word * num2);
word * num_not(word * heap, word * num);
word * num_ldr(word * heap, word * num);
void num_store(word * num1, word * num2);
word * num_mul(word * heap, word * num1, word * num2);
word * num_div(word * heap, word * num1, word * num2);
word * str_to_num(word * heap, word * num);
void print_num(word * num);
word * word_to_num(word * heap, word w);


//util.c
word max(sword x, sword y);
word umax(word x, word y);
word min(sword x, sword y);
word umin(word x, word y);
word strlen(const word * str);
word strncmp(uint8_t * s1, uint8_t * s2, word len);
void * memcpy(void * dest, const void * src, size_t n);
void * memset(void * str, int c, size_t n);
word memcmp(word * m1, word * m2, word len);
word atomic_cas(word * ptr, word cmp, word new);
word nat_pow(word base, word exp);
word abs(word x);
word arithmetic_shift_right(word val, word n);
void outb(uint16_t port, uint8_t val);
EFI_STATUS get_char(EFI_INPUT_KEY * input_key);


//lisp.c
word * init_machine(word * heap, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE * SystemTable);
word * tokenize(word * heap, word * code, word code_len);
