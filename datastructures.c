#include "config.h"

//NOTE: Some of the datastructures here depend on the memory allocator leaving the size of the allocated memory just before the mem.

// Note: Use black magic to get maximum size from the umds associated with the object when expanding; No sense consuming another word.
// TODO: Refcount attributes, e.g. cyclic (also add in manual ability to deallocate or cycle check)
typedef struct object{
  word max_sz; //(in umds, includes size of obj, must be here)
  word refcount;
  word type;
  word size;
  word contents[];
}Object;

const word obj_sz = sizeof(Object)/sizeof(word);


word * object(word * heap, word * type, word size, word * contents, word n_words){
  word * mem = alloc(heap, size + obj_sz - 1);
  if (!mem){return (word*)0;}
  Object * obj = (Object*)(mem - 1);
  obj->refcount = 1;
  obj->type = (word)type;
  obj->size = n_words;
  if (contents){
    for (word i = 0; i < n_words; ++i){
      obj->contents[i] = contents[i];
    }
  }
  return (word*)obj;
}


void object_delete(word * heap, word * obj){
  free(heap, obj + 1);
}


typedef struct Array_DS{
  word mem_sz;
  word used_sz;
  word item_sz;
}Array;

const word Array_bsz = sizeof(Array)/sizeof(word) - 1;


word * array(word * heap, word size, word item_sz){
  word * mem = alloc(heap, size * item_sz + Array_bsz);
  if (!mem){return (word*)0;}
  Array * arr = (Array*)(mem - 1);
  arr->used_sz = 0;
  arr->item_sz = item_sz;
  return mem + Array_bsz;
}


word * array_find(word * arr, word * start, word * item, word (*eq_fn)(word *, word *, word *), word * extra_params){
    Array * array = (Array*)(arr - Array_bsz - 1);
    word item_sz = array->item_sz;
    for (word * ptr = start; ptr < arr + array_len(arr) * item_sz; ptr += item_sz){
      if (eq_fn(item, ptr, extra_params)){
	return ptr;
      }
    }
    return (word*)0;
}


word * array_append(word * heap, word * arr, word * item){
  word * mem_addr = arr - Array_bsz;
  Array * handle = (Array*)(mem_addr - 1);
  word true_mem_sz = get_mem_sz(mem_addr);
  word mem_sz = true_mem_sz - Array_bsz;
  word used_sz = handle->used_sz;
  word item_sz = handle->item_sz;

  if (used_sz + item_sz > mem_sz){
    mem_addr = realloc(heap, mem_addr, true_mem_sz + mem_sz/2 + item_sz);
    if (!mem_addr){return (word*)0;}
    arr = mem_addr + Array_bsz;
    handle = (Array*)(mem_addr - 1);
  }
  for (word i = 0; i < item_sz; ++i){
    *(arr + used_sz + i) = *(item + i);
  }
  handle->used_sz += item_sz;

  return arr;
}


void array_delete(word * heap, word * arr){
  arr -= Array_bsz;
  free(heap, arr);
}


word array_capacity(word * arr){
  Array * array = (Array*)(arr - Array_bsz - 1);
  return array->mem_sz - Array_bsz - 1;
}


word array_len(word * arr){
  Array * array = (Array*)(arr - Array_bsz - 1);
  return array->used_sz / array->item_sz;
}
