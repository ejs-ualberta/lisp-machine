#include "config.h"

word true_num_alloced = 0;
word gc_num_alloced = 0;
word * gc_set;

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
const word min_alloc_sz = sizeof(AVL_Node)/sizeof(word);


word * init_heap(word * heap_start, word heap_sz){
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


word * gc_init(word * heap){
  word size = 1;
  word * mem = alloc(heap, size + obj_sz - 1);
  if (!mem){return (word*)0;}
  Object * obj = (Object*)(mem - 1);
  obj->refcount = 1;
  obj->type = (word)set_type;
  ((Object*)set_type)->refcount += 1;
  obj->size = size;
  obj->contents[0] = 0;
  gc_set = (word*)obj;
  return (word*)obj;
}


word * alloc(word * heap, word mem_sz){
  extern void avl_move(word ** tr, word * dest, word * src);
  mem_sz += umds_sz;
  if (mem_sz < min_alloc_sz){mem_sz = min_alloc_sz;}

  hds * h_info = (hds*)heap;
  word ** tr = (word**)&(h_info->fmds_ptr);
  word * mem_ptr = (word*)h_info->fmds_ptr;
  AVL_Node * min_ge = (AVL_Node*)avl_min_ge(mem_ptr, mem_sz);
  if (!min_ge){return 0;}
  word old_sz = min_ge->data;
  //print_avl(heap[3], 0, 2);
  word * deleted = _avl_delete(tr, (word*)min_ge, &avl_mem_cmp);
  if (deleted != (word*)min_ge){avl_move(tr, deleted, (word*)min_ge);} // Move successor of min_ge back to where it was.
  if (old_sz >= mem_sz + min_alloc_sz){
    word new_sz = old_sz - mem_sz;
    word * new_ptr = (word*)min_ge + mem_sz;
    _avl_insert(tr, new_ptr, new_sz, &avl_mem_cmp);
    //print_avl(heap[3], 0, 2);nl(1);
  }else{
    mem_sz = old_sz;
  }

  umds * mem = (umds*)(min_ge);
  word * addr = (word*)&mem->mem;
  mem->mem_sz = mem_sz;
  //print_cstr("a");print_uint(&mem->mem, 16,0);spc(1);
  //array_append(heap, alloc_buf + 3, (word*)&addr);
  ++true_num_alloced;
  return addr;
}

//TODO: gc_alloc and gc_free
word * gc_alloc(word * heap, word n){
  word * addr = alloc(heap, n);
  word ** tr = (word**)&((Object*)gc_set)->contents;
  word cond = avl_insert(heap, tr, (word)addr, &avl_basic_cmp);
  if (cond){
    free(heap, addr);
    return 0;
  }
  ++gc_num_alloced;
  return addr;
}


void free(word * heap, word * addr){
  //print_avl(heap[3], 0, 2);nl(1);
  hds * h_info = (hds*)heap;
  if (addr < (word*)h_info + hds_sz || addr >= (word*)h_info + h_info->heap_end){
    return;
  }
  word ** tr = (word**)&(h_info->fmds_ptr);
  word * freed_mem = addr - umds_sz;
  umds * mem_obj = (umds*)(addr - umds_sz);
  word fm_sz = mem_obj->mem_sz;
  avl_merge(tr, freed_mem, fm_sz);
  --true_num_alloced;
  //print_cstr("f");print_uint(addr, 16, 0);spc(1);
  /* for (word i = 0; i < 1024; ++i){ */
  /*   if ((alloc_buf + 3)[i] == addr){ */
  /*     (alloc_buf + 3)[i] = 0; */
  /*     break; */
  /*   } */
  /* } */
}


void gc_free(word * heap, word * addr){
  word * a = set_remove(heap, gc_set, addr, &avl_basic_cmp);
  if (a){
    free(heap, addr);
  };
  --gc_num_alloced;
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


word * gc_realloc(word * heap, word * addr, word mem_sz){
  word * a = set_remove(heap, gc_set, addr, &avl_basic_cmp);
  //if (!a){return 0;}
  word * res = realloc(heap, addr, mem_sz);
  if (avl_insert(heap, (word**)&(((Object*)gc_set)->contents), (word)addr, &avl_basic_cmp)){
    return 0;
  }
  return res;
}


word * gc_free_all(word * heap, word * set){
  word ** tr = (word**)&(((Object*)set)->contents);
  while (*tr){
    AVL_Node * root = (AVL_Node*)*tr;
    word * addr = (word*)avl_delete(heap, tr, root->data, avl_basic_cmp);
    gc_free(heap, addr);
  }
  free(heap, set + 1);
}


word get_mem_sz(word * addr){
  umds * used_mem = (umds*)(addr - umds_sz);
  return used_mem->mem_sz - umds_sz;
}
