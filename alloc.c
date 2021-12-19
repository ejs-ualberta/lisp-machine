#include "config.h"

//TODO: Generational gc

const word refcount_mask = (word)-1 >> (sizeof(word) * 4);
const word visited_mask = (word)1 << (sizeof(word) * 8 - 1);

/* word alloc_buf[2048] = {2048, 0, 1, 0}; */

const word umds_sz = sizeof(umds)/sizeof(word);
const word hds_sz = sizeof(hds)/sizeof(word);
const word min_alloc_sz = sizeof(AVL_Node) / sizeof(word);


word get_rc(word *obj) {
  Object * o = (Object*)obj;
  return o->refcount & refcount_mask;
}


word get_tmp_rc(word * obj) {
  Object * o = (Object*)obj;
  return (o->refcount & ~visited_mask) >> (sizeof(word) * 4);
}


void _set_tmp_rc(word * obj, word val){
  Object * o = (Object*)obj;
  o->refcount &= refcount_mask;
  o->refcount |= val << (sizeof(word) * 4);
}


void set_tmp_rc(word * obj, word val){
  Object * o = (Object*)obj;
  o->refcount &= refcount_mask | visited_mask;
  o->refcount |= val << (sizeof(word) * 4);
}


word visited(word * obj){
  Object * o = (Object*)obj;
  return o->refcount & visited_mask;
}


void set_visited(word * obj, word val){
  Object * o = (Object*)obj;
  o->refcount &= (word)-1 >> 1;
  o->refcount |= val << (sizeof(word)*8 - 1);
}


word get_mem_sz(word * addr){
  umds * used_mem = (umds*)(addr - umds_sz);
  return used_mem->mem_sz - umds_sz;
}


word _check_gc(word * tree) {
  if (!tree){return 0;}
  AVL_Node * node = (AVL_Node*)tree;
  AVL_Node * left = (AVL_Node*)node->left;
  AVL_Node * right = (AVL_Node*)node->right;
  word ret = 1;
  Object * o = (Object*)((word*)node->data - 1);
  uart_print_uint((word)o->refcount, 16);uart_puts(" ");uart_print_uint((word)o, 16);uart_puts(" ");rec_obj_print((word*)o);uart_puts("\n");
  ret += _check_gc((word*)left);
  ret += _check_gc((word*)right);
  return ret;
}


word check_gc(word * gc_set) {
  word * tr = (word*)((Object*)gc_set)->contents[0];
  return _check_gc(tr);
}


word * init_heap(word * heap_start, word heap_sz){
  if ((word)heap_start % sizeof(word)){return 0;}
  if (heap_sz < min_alloc_sz + hds_sz){return 0;}

  hds * heap = (hds*)heap_start;
  heap->heap_end = (word)(heap_start + heap_sz);
  heap->prev = 0;
  heap->next = 0;
  heap->fmds_ptr = (word)(heap_start + hds_sz);

  heap->true_num_alloced = 0;
  heap->gc_num_alloced = 0;
  heap->words_used = 0;

  word * mem = heap_start + hds_sz;
  _avl_insert((word**)&(heap->fmds_ptr), (word*)(heap->fmds_ptr), heap_sz - hds_sz, &avl_basic_cmp);

  word size = 1;
  word * o = (word*)alloc((word*)heap, size + obj_sz - 1);
  if (!mem){return (word*)0;}
  Object * obj = (Object*)(o - 1);
  obj->refcount = 1;
  obj->size = size;
  obj->contents[0] = 0;
  heap->gc_set = (word*)obj;

  init_types((word*)heap);
  ((Object*)heap->gc_set)->type = (word)set_type;
  ((Object*)set_type)->refcount += 1;

  return heap_start;
}


word * alloc(word * heap, word mem_sz){
  extern void avl_move(word ** tr, word * dest, word * src);
  mem_sz += umds_sz;
  if (mem_sz < min_alloc_sz){mem_sz = min_alloc_sz;}

  hds * h_info = (hds*)heap;
  word ** tr = (word**)&(h_info->fmds_ptr);
  word * mem_ptr = (word*)h_info->fmds_ptr;
  AVL_Node * min_ge = (AVL_Node*)avl_min_ge(mem_ptr, mem_sz);
  if (!min_ge){
    return 0;
  }
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
  /* array_append(heap, alloc_buf + 3, (word*)&addr); */
  ++h_info->true_num_alloced;
  h_info->words_used += mem_sz;
  return addr;
}


void check_heap_capacity(word * heap){
  static word gc_lim = 0;
  hds * h_info = (hds*)heap;
  word total_mem = ((word*)(h_info->heap_end) - heap);
  word n = 8;
  word ll = total_mem/n;
  word ul = ((n-1) * ll) / 2;
  //check_gc(gc_set);
  if (h_info->words_used >= gc_lim){
    gc_collect(heap);
    gc_lim = min(3 * max(ll, h_info->words_used) / 2, ul);
  }
}


word * gc_alloc(word * heap, word n){
  hds * h_info = (hds*)heap;
  word * addr = alloc(heap, n);
  if (!addr){return 0;}
  Object * gc_set = (Object*)h_info->gc_set;
  word ** tr = (word**)&gc_set->contents[0];
  word cond = avl_insert(heap, tr, (word)addr, &avl_basic_cmp);
  if (cond){
    free(heap, addr);
    return 0;
  }
  ++h_info->gc_num_alloced;
  return addr;
}


void free(word * heap, word * addr){
  //print_avl(heap[3], 0, 2);nl(1);
  hds * h_info = (hds*)heap;
  //print_avl(((Object*)(h_info->gc_set))->contents[0]);uart_puts("\n\n");
  if (addr < (word*)h_info + hds_sz || addr >= (word*)h_info + h_info->heap_end){
    return;
  }
  word ** tr = (word**)&(h_info->fmds_ptr);
  word * freed_mem = addr - umds_sz;
  umds * mem_obj = (umds*)(addr - umds_sz);
  word fm_sz = mem_obj->mem_sz;
  avl_merge(tr, freed_mem, fm_sz);
  --h_info->true_num_alloced;
  h_info->words_used -= fm_sz;
  //print_cstr("f");print_uint(addr, 16, 0);spc(1);
  /* for (word i = 0; i < 2048-3; ++i){ */
  /*   if ((alloc_buf + 3)[i] == addr){ */
  /*     (alloc_buf + 3)[i] = 0; */
  /*     break; */
  /*   } */
  /* } */
}


void gc_free(word * heap, word * addr){
  hds * h_info = (hds*)heap;
  word * gc_set = (word*)h_info->gc_set;
  word * a = set_remove(heap, gc_set, addr, &avl_basic_cmp);
  if (a){
    free(heap, addr);
  };
  --h_info->gc_num_alloced;
}


//TODO: make this so that if the realloc fails it doesn't free the memory given.
word * realloc(word * heap, word * addr, word mem_sz){
  word * new_addr = alloc(heap, mem_sz);
  if (!new_addr){return 0;}
  umds * old_obj = (umds*)(addr - umds_sz);
  for (word i = 0; i < umin(old_obj->mem_sz - 1, mem_sz); ++i){
    new_addr[i] = addr[i];
  }
  free(heap, addr);
  return new_addr;
}


word * gc_realloc(word * heap, word * addr, word mem_sz){
  word * new_addr = gc_alloc(heap, mem_sz);
  umds * old_obj = (umds*)(addr - umds_sz);
  for (word i = 0; i < umin(old_obj->mem_sz - 1, mem_sz); ++i){
    new_addr[i] = addr[i];
  }
  gc_free(heap, addr);
  return new_addr;
}


/* void check_tc_tr(word ** s, word ** pending, word ** tr, word * tree) { */
/*   if (!tree){return;} */
/*   void check_tc(word ** s, word **  pending, word ** tr, word * obj); */
/*   AVL_Node * node = (AVL_Node*)tree; */
/*   AVL_Node * left = (AVL_Node*)node->left; */
/*   AVL_Node * right = (AVL_Node*)node->right; */
/*   word * obj = (word*)node->data - 1; */
/*   check_tc(s, pending, tr, (word*)node->data); */
/*   check_tc_tr(s, pending, tr, (word*)left); */
/*   check_tc_tr(s, pending, tr, (word*)right); */
/*   set_visited(obj, 0); */
/* } */


/* void check_tc(word ** s, word ** pending, word ** tr, word * obj){ */
/*   Object * o = (Object*)obj; */
/*   if (!obj){ */
/*     print_cstr("Null Object "); */
/*     return; */
/*   }else if (visited(obj)){ */
/*     return; */
/*   }else{ */
/*     set_visited(obj, 1); */
/*   } */
/*   word * node = avl_find(s, (word)(obj + 1), &avl_basic_cmp); */
/*   if (!node){ */
/*     node = avl_find(pending, (word)(obj + 1), &avl_basic_cmp); */
/*     if (node){ */
/*       print_uint(visited(obj), 16, 0);spc(1);print_uint(get_tmp_rc(obj), 16, 0);spc(1);print_uint(get_rc(obj), 16, 0);spc(1);rec_obj_print(obj);nl(1); */
/*       return; */
/*     }else{ */
/*       node = avl_find(tr, (word)(obj + 1), &avl_basic_cmp); */
/*       if (!node){ */
/* 	print_cstr("Missing Node ");rec_obj_print(obj);nl(1); */
/*       }else{ */
/* 	print_cstr("Wrong Node Location "); */
/*       } */
/*     } */
/*   } */
/*   word * type = (word*)o->type; */
/*   if (!obj_cmp(type, string_type) || !obj_cmp(type, num_type)){ */
/*     return; */
/*   }else if (!obj_cmp(type, set_type)){ */
/*     check_tc_tr(s, pending, tr, (word*)o->contents[0]); */
/*   }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type) || !obj_cmp(type, subarray_type)){ */
/*     for (word i = 0; i < o->size; ++i){ */
/*       check_tc(s, pending, tr, (word*)o->contents[i]); */
/*     } */
/*   }else if (!obj_cmp(type, array_type)){ */
/*     check_tc(s, pending, tr, (word*)o->contents[0]); */
/*   } */
/* } */


/* void checkall_tc(word **s, word ** pending, word ** tr, word * tree) { */
/*   if (!tree){return;} */
/*   AVL_Node * node = (AVL_Node*)tree; */
/*   AVL_Node * left = (AVL_Node*)node->left; */
/*   AVL_Node * right = (AVL_Node*)node->right; */
/*   check_tc(s, pending, tr, (word*)node->data - 1); */
/*   checkall_tc(s, pending, tr, (word*)left); */
/*   checkall_tc(s, pending, tr, (word*)right); */
/* } */


void mark_tc_tr(word * heap, word * q, word * tree, word cond){
  if (!tree){return;}
  void _mark_tc(word * heap, word * q, word * obj, word cond);
  AVL_Node * node = (AVL_Node*)tree;
  AVL_Node * left = (AVL_Node*)node->left;
  AVL_Node * right = (AVL_Node*)node->right;
  _mark_tc(heap, q, (word*)node->data, cond);
  mark_tc_tr(heap, q, (word*)left, cond);
  mark_tc_tr(heap, q, (word*)right, cond);
  return;
}


void _mark_tc(word * heap, word * q, word * obj, word cond){
  Object * o = (Object*)obj;
  if (!o){return;}
  word tmp_rc = get_tmp_rc(obj);

  if (cond){
    if (visited(obj)){
      set_tmp_rc(obj, tmp_rc + 1);
      return;
    }
    queue_push(heap, q, (word)obj);
    set_visited(obj, 1);
  }else{
    if (!visited(obj)){
      return;
    }
    set_visited(obj, 0);
  }

  
  set_tmp_rc(obj, cond);
  word * type = (word*)o->type;
  if (!obj_cmp(type, string_type) || !obj_cmp(type, num_type)){
    return;
  }else if (!obj_cmp(type, set_type)){
    mark_tc_tr(heap, q, (word*)o->contents[0], cond);
  }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type) || !obj_cmp(type, subarray_type)){
    for (word i = 0; i < o->size; ++i){
      _mark_tc(heap, q, (word*)o->contents[i], cond);
    }
  }else if (!obj_cmp(type, array_type)){
    _mark_tc(heap, q, (word*)o->contents[0], cond);
  }
}


word * mark_tc(word * heap, word * obj, word cond){
  word * q = 0;
  if (cond){q = queue(heap);}
  _mark_tc(heap, q, obj, cond);
  if (cond){
    word t_rc = get_tmp_rc(obj);
    set_tmp_rc(obj, t_rc - 1);
  }
  return q;
}


void decrease_refcs(word *obj) {
  if (!obj){return;}
  Object * o = (Object*)obj;
  //set_tmp_rc(obj, get_tmp_rc(obj) - 1);
  --o->refcount;
}


word sweep_tr(word * heap, word * tree, word * q) {
  word sweep(word * heap, word * obj, word * q);
  if (!tree){return 0;}
  AVL_Node * node = (AVL_Node*)tree;
  AVL_Node * left = (AVL_Node*)node->left;
  AVL_Node * right = (AVL_Node*)node->right;
  decrease_refcs((word*)node->data);
  word ret = sweep(heap, (word*)node->data, q);
  free(heap, (word*)node);
  ret += sweep_tr(heap, (word*)left, q);
  ret += sweep_tr(heap, (word*)right, q);
  return ret;
}


word sweep(word * heap, word * obj, word * q){
  Object * o = (Object*)obj;
  if (!o || !visited(obj)){return 0;}

  // Return if object has already been freed or is in the process of being freed
  hds * h_info = (hds*)heap;
  Object * gc_set = (Object*)h_info->gc_set;
  word ** tr = (word**)&(((Object*)gc_set)->contents);
  AVL_Node * node = (AVL_Node*)avl_find(tr, (word)(obj+1), &avl_basic_cmp);
  if (!node){return 0;}

  word ret = 0;
  word * mem = (word*)obj + 1;
  avl_delete(heap, tr, (word)mem, &avl_basic_cmp);
  queue_push(heap, q, (word)obj);
  word * type = (word*)o->type;
  if (!obj_cmp(type, string_type) || !obj_cmp(type, num_type)){
    ret = 1;
  }else if (!obj_cmp(type, set_type)){
    ret += sweep_tr(heap, (word*)o->contents[0], q);
  }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type) || !obj_cmp(type, subarray_type)){
    for (word i = 0; i < o->size; ++i){
      decrease_refcs((word*)o->contents[i]);
      ret += sweep(heap, (word*)o->contents[i], q);
    }
  }else if (!obj_cmp(type, array_type)){
    decrease_refcs((word*)o->contents[0]);
    ret += sweep(heap, (word*)o->contents[0], q);
  }

  ret += 1;
  return ret;
}


void _gc_collect(word *heap, word *tree, word * q) {
  if (!tree){return;}
   AVL_Node * node = (AVL_Node*)tree;
  AVL_Node * left = (AVL_Node*)node->left;
  AVL_Node * right = (AVL_Node*)node->right;
  _gc_collect(heap, (word*)left, q);
  _gc_collect(heap, (word*)right, q);
  Object * data = (Object*)((word*)node->data - 1);
  if (data->type && !obj_cmp((word*)data->type, function_type)){
    queue_push(heap, q, (word)data);
  }
}


/* void mark_tc_noq(word ** tr, word ** dest, word ** pending){ */
/*   while(*tr){ */
/*     word * data = (word*)((AVL_Node*)*tr)->data; */
/*     AVL_Node * node = (AVL_Node*)_avl_delete(tr, *tr, &avl_basic_cmp); */
/*     _avl_insert(dest, (word*)node, (word)data, &avl_basic_cmp); */
/*   } */
/* } */


void gc_collect(word * heap){
  hds * h_info = (hds*)heap;
  word * gc_set = h_info->gc_set;
  word ** tr = (word**)&((Object*)gc_set)->contents;
  word * q0 = queue(heap);
  _gc_collect(heap, *tr, q0);

  word * data;
  //check_gc(gc_set);
  while((data = (word*)queue_pop(heap, q0))){
    //continue if obj has already been freed
    tr = (word**)&(((Object*)gc_set)->contents);
    AVL_Node * node = (AVL_Node*)avl_find(tr, (word)(data+1), &avl_basic_cmp);
    if (!node){continue;}

    word * q = mark_tc(heap, data, 1);
    word * obj;
    while ((obj = (word*)queue_pop(heap, q))){
      if (get_tmp_rc(obj) < get_rc(obj)){
	mark_tc(heap, obj, 0);
      }
    }
    sweep(heap, data, q);
    //queue_print(q);uart_puts("\n\n");
    while ((obj = (word*)queue_pop(heap, q))){
      //queue_print(q);uart_puts("\n\n");
      object_delete(heap, (word*)((Object*)obj)->type);
      //if (!obj_cmp(((Object*)obj)->type, function_type)){rec_obj_print(obj);nl(1);}
      free(heap, (word*)obj + 1);
      --h_info->gc_num_alloced;
    }
    free(heap, q);
  }
  free(heap, q0);
  //check_gc(gc_set);
}


/* void gc_collect(word * heap, word * gc_set){ */
/*   hds * h_info = (hds*)heap; */
/*   word ** tr = (word**)&(((Object*)gc_set)->contents); */
/*   word * dest_set = 0; */
/*   word * pending_set = 0; */

/*   mark_all(*tr, 1); */
/*   //_check_gc(*tr); */
/*   while (*tr){ */
/*     AVL_Node * node = (AVL_Node*)*tr; */
/*     word * obj = (word*)node->data - 1; */
/*     word cond = get_tmp_rc(obj) < get_rc(obj); */
/*     node = (AVL_Node*)_avl_delete(tr, *tr, &avl_basic_cmp); */
/*     if (cond || !visited(obj)){ */
/*       mark_tc(obj, 0); */
/*       _avl_insert(&dest_set, (word*)node, (word)(obj + 1), &avl_basic_cmp); */
/*       continue; */
/*       //check_tc(&dest_set, &pending_set, tr, obj); */
/*     } */
/*     _avl_insert(&pending_set, (word*)node, (word)(obj + 1), &avl_basic_cmp); */
/*   } */
/*   //checkall_tc(&dest_set, &pending_set, tr, dest_set); */

/*   word ** pp = &pending_set; */
/*   while (*pp){ */
/*     AVL_Node * node = (AVL_Node*)*pp; */
/*     word * obj = (word*)node->data - 1; */
/*     //Object * obj = (Object*)((word*)node->data - 1); */
/*     node = (AVL_Node*)_avl_delete(pp, pending_set, &avl_basic_cmp); */
/*     //print_uint(obj, 16, 0);nl(1); */
/*     if (!visited(obj)){ */
/*       //rec_obj_print(obj); */
/*       _avl_insert(&dest_set, (word*)node, (word)(obj + 1), &avl_basic_cmp); */
/*       continue; */
/*     } */
/*     //free(heap, (word*)obj + 1); */
/*     //free(heap, (word*)node); */
/*     //object_delete(heap, (word*)((Object*)obj)->type); */
/*     //_check_gc(pending_set);nl(2); */
/*     //_avl_insert(tr, (word*)node, (word)(obj + 1), &avl_basic_cmp); */
/*   } */
/*   checkall_tc(tr, &pending_set, &dest_set, *tr); */
/*   //_check_gc(*tr); */
/*   *tr = dest_set; */
/* } */


/* void gc_collect(word * heap, word * gc_set){ */
/*   hds * h_info = (hds*)heap; */
/*   word ** tr = (word**)&(((Object*)gc_set)->contents); */
/*   word * dest_set = 0; */
/*   word * pending_set = 0; */

/*   mark_all(*tr, 1); */
/*   //_check_gc(*tr); */
/*   while (*tr){ */
/*     AVL_Node * node = (AVL_Node*)*tr; */
/*     word * obj = (word*)node->data - 1; */
/*     word cond = get_tmp_rc(obj) < get_rc(obj); */
/*     if (cond || !visited(obj)){ */
/*       move_tc(obj, tr, &pending_set, &dest_set); */
/*       //check_tc(&dest_set, &pending_set, tr, obj); */
/*     }else{ */
/*       node = (AVL_Node*)_avl_delete(tr, *tr, &avl_basic_cmp); */
/*       _avl_insert(&pending_set, (word*)node, (word)(obj + 1), &avl_basic_cmp); */
/*     } */
/*   } */
/*   //checkall_tc(&dest_set, &pending_set, tr, dest_set); */
/*   //_check_gc(dest_set); */

/*   word ** pp = &pending_set; */
/*   while (*pp){ */
/*     AVL_Node * node = (AVL_Node*)*pp; */
/*     word * obj = (word*)node->data - 1; */
/*     //Object * obj = (Object*)((word*)node->data - 1); */
/*     node = (AVL_Node*)_avl_delete(pp, pending_set, &avl_basic_cmp); */
/*     //print_uint(obj, 16, 0);nl(1); */
/*     if (!node || !obj){ */
/*       print_cstr("Null Node\n"); */
/*       continue; */
/*     }else if (get_tmp_rc(obj) < get_rc(obj) || !visited(obj)){ */
/*       rec_obj_print(obj); */
/*       continue; */
/*     } */
/*     free(heap, (word*)node); */
/*     object_delete(heap, (word*)((Object*)obj)->type); */
/*     //_check_gc(pending_set);nl(2); */
/*     //free(heap, (word*)obj + 1); */
/*   } */
/*   *tr = dest_set; */
/* } */
