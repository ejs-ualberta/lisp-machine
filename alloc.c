#include "config.h"

typedef struct used_mem_datastructure{
  word mem_sz;
  word mem[];
} umds;

typedef struct heap_datastructure{
  word heap_end;
  word prev;
  word next;
  word fmds_ptr;
} hds;

const word umds_sz = sizeof(umds)/sizeof(word);
const word hds_sz = sizeof(hds)/sizeof(word);
word min_alloc_sz = 0; // Note: This is ugly and stupid because according to C avl_node_sz is not a compile time constant. (set in init_heap)


word * init_heap(word * heap_start, word heap_sz){
  min_alloc_sz = avl_node_sz;
  if ((word)heap_start % sizeof(word)){return 0;}
  if (heap_sz < min_alloc_sz + hds_sz){return 0;}

  hds * heap = (hds*)heap_start;
  heap->heap_end = (word)(heap_start + heap_sz);
  heap->prev = 0;
  heap->next = 0;
  heap->fmds_ptr = (word)(heap_start + hds_sz);

  word * mem = heap_start + hds_sz;
  _avl_insert((word**)&(heap->fmds_ptr), (word*)(heap->fmds_ptr), heap_sz - hds_sz, &avl_basic_cmp);

  return heap_start;
}


word * alloc(word * heap, word mem_sz){
  mem_sz += umds_sz;
  if (mem_sz < min_alloc_sz){mem_sz = min_alloc_sz;}

  hds * h_info = (hds*)heap;
  word ** tr = (word**)&(h_info->fmds_ptr);
  word * mem_ptr = (word*)h_info->fmds_ptr;
  word min_ge = avl_min_ge(mem_ptr, mem_sz);
  if (!min_ge){return 0;}

  mem_ptr = _avl_delete(tr, min_ge, &avl_basic_cmp);
  if (min_ge >= mem_sz + min_alloc_sz){
    word os = min_ge - mem_sz;
    word * new_ptr = mem_ptr + mem_sz;
    _avl_insert(tr, new_ptr, os, &avl_basic_cmp);
  }else{
    mem_sz = min_ge;
  }

  umds * mem = (umds*)(mem_ptr);
  mem->mem_sz = mem_sz;
  return (word*)&(mem->mem);
}


void free(word * heap, word * addr){
  hds * h_info = (hds*)heap;
  if (addr < (word*)h_info + hds_sz || addr >= (word*)h_info + h_info->heap_end){
    return;
  }
  word ** tr = (word**)&(h_info->fmds_ptr);
  word * freed_mem = addr - umds_sz;
  umds * mem_obj = (umds*)(addr - umds_sz);
  word fm_sz = mem_obj->mem_sz;
  avl_merge(tr, freed_mem, fm_sz);
}


//TODO: make this so that if the realloc fails it doesn't free the memory given.
word * realloc(word * heap, word * addr, word mem_sz){
  word * new_addr = alloc(heap, mem_sz);
  if (!new_addr){return 0;}
  umds * old_obj = (umds*)(addr - umds_sz);
  for (word i = 0; i < old_obj->mem_sz; ++i){
    new_addr[i] = addr[i];
  }
  free(heap, addr);
  return new_addr;
}


word get_mem_sz(word * addr){
  umds * used_mem = (umds*)(addr - umds_sz);
  return used_mem->mem_sz - umds_sz;
}
