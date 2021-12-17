#pragma once
#include <stdint.h>
#include <stddef.h>

typedef unsigned long long word;
typedef long long sword;


extern volatile unsigned char _data;
extern volatile unsigned char _end;
extern word * global_heap_start;
extern word global_heap_size;


// datastructures.c
extern word * num_type;
extern word * string_type;
extern word * array_type;
extern word * subarray_type;
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

typedef struct array_ds{
  word mem_sz;
  word used_sz;
  word item_sz;
}Array;

typedef struct avl_node{
  word prev;
  word left;
  word right;
  word data;
}AVL_Node;

typedef struct queue{
  word first;
  word last;
}Queue;

typedef struct link{
  word data;
  word next;
  word prev;
}Link;

typedef struct ringbuf{
  word * buf;
  word head;
  word tail;
}RingBuf;

typedef struct c_fifo{
  RingBuf * A;
  RingBuf * B;
  word a_lck;
  word b_lck;
  word tag;
  word curr_tag;
}ConcurrentFIFO;


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
word * array_resize(word * heap, word * arr, word new_size);
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
void print_avl(word * tree);
word * queue(word * heap);
word * queue_push(word * heap, word * queue, word data);
word queue_last(word * heap, word * queue);
word queue_pop(word * heap, word * queue);
void queue_print(word * queue);
word * ring_buf(word * heap, word size, word item_sz);
word ring_buf_isempty(word * rb);
word ring_buf_isfull(word * rb);
word ring_buf_item_sz(word * ringbuf);
void ring_buf_add(word * rb, word * item);
void ring_buf_pop(word * rb, word * item);
void ring_buf_delete(word * heap, word * rb);
word * concurrent_fifo(word * heap, word primary_capacity, word secondary_capacity, word item_sz);
word concurrent_fifo_isempty(word * c_fifo);
word concurrent_fifo_isfull(word * c_fifo);
word concurrent_fifo_item_sz(word * c_fifo);
word concurrent_fifo_add(word * c_fifo, word * item);
word concurrent_fifo_pop(word * c_fifo, word * item);
void concurrent_fifo_delete(word * heap, word * c_fifo);
void init_types(word * heap);
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
word * cstr_to_string(word * heap, char * str);
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
word * str_to_num(word * heap, word * num);
void print_num(word * num);
word * word_to_num(word * heap, word w);


// alloc.c
typedef struct used_mem_datastructure{
  word mem_sz;
  word mem[];
} umds;

typedef struct heap_datastructure{
  word heap_end;
  word prev;
  word next;
  word fmds_ptr;
  word * gc_set;
  word true_num_alloced;
  word gc_num_alloced;
  word words_used;
} hds;

extern const word hds_sz;
word get_mem_sz(word * addr);
word check_gc(word * gc_set);
word * init_heap(word * heap_start, word heap_sz);
word * gc_init(word * heap);
word * alloc(word * heap, word mem_sz);
void check_heap_capacity(word * heap);
word * gc_alloc(word * heap, word n);
void free(word * heap, word * addr);
void gc_free(word * heap, word * addr);
word * realloc(word * heap, word * addr, word mem_sz);
word * gc_realloc(word * heap, word * addr, word mem_sz);
void gc_collect(word * heap);


// util.c
void uart_send(unsigned int c);
char uart_getc();
void uart_puts(char * s);
void uart_print_uint(word val, word base);
void uart_padded_uint(word val, word base, word padding);
int uintn_to_str(uint16_t * buf, word buf_sz, word num, word base);
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
void breakp();


// asm.c
word * compile(word * heap, word * code, word code_sz);
word run(word * exception_fifo, word * bytecode);
