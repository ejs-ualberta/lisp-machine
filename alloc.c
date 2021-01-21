#include "config.h"

word true_num_alloced = 0;
word gc_num_alloced = 0;
word * gc_set;

const word refcount_mask = (word)-1 >> (sizeof(word) * 4);
const word visited_mask = (word)1 << (sizeof(word) * 8 - 1);

/* word alloc_buf[2048] = {2048, 0, 1, 0}; */

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
const word min_alloc_sz = sizeof(AVL_Node) / sizeof(word);


word get_rc(word *obj) {
  Object * o = (Object*)obj;
  return o->refcount & refcount_mask;
}


word get_tmp_rc(word * obj) {
  Object * o = (Object*)obj;
  return (o->refcount & ~visited_mask) >> (sizeof(word) * 4);
}


void _set_tmp_rc(word *obj, word val) {
  Object * o = (Object*)obj;
  o->refcount &= refcount_mask;
  o->refcount |= val << (sizeof(word) * 4);
}


void set_tmp_rc(word *obj, word val) {
  Object * o = (Object*)obj;
  o->refcount &= refcount_mask | visited_mask;
  o->refcount |= val << (sizeof(word) * 4);
}


word visited(word *obj) {
  Object * o = (Object*)obj;
  return o->refcount & visited_mask;
}


void set_visited(word *obj, word val){
  Object * o = (Object*)obj;
  o->refcount &= (word)-1 >> 1;
  o->refcount |= val << (sizeof(word)*8 - 1);
}


word get_mem_sz(word * addr){
  umds * used_mem = (umds*)(addr - umds_sz);
  return used_mem->mem_sz - umds_sz;
}


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
  /* array_append(heap, alloc_buf + 3, (word*)&addr); */
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
  /* for (word i = 0; i < 2048-3; ++i){ */
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
  for (word i = 0; i < umin(old_obj->mem_sz - 1, mem_sz); ++i){
    new_addr[i] = addr[i];
  }
  free(heap, addr);
  return new_addr;
}


//TODO: make this better.
word * gc_realloc(word * heap, word * addr, word mem_sz){
  /* word * a = set_remove(heap, gc_set, addr, &avl_basic_cmp); */
  /* word * res = realloc(heap, addr, mem_sz); */
  /* if (avl_insert(heap, (word**)&(((Object*)gc_set)->contents), (word)addr, &avl_basic_cmp)){ */
  /*   return 0; */
  /* } */
  word * new_addr = gc_alloc(heap, mem_sz);
  umds * old_obj = (umds*)(addr - umds_sz);
  for (word i = 0; i < umin(old_obj->mem_sz - 1, mem_sz); ++i){
    new_addr[i] = addr[i];
  }
  gc_free(heap, addr);
  return new_addr;
}


word _check_gc(word * tree) {
  if (!tree){return 0;}
  AVL_Node * node = (AVL_Node*)tree;
  AVL_Node * left = (AVL_Node*)node->left;
  AVL_Node * right = (AVL_Node*)node->right;
  word ret = 1;
  Object * o = (Object*)((word*)node->data - 1);
  if (1){
      print_uint((word)o->refcount, 16, 0);spc(1);print_uint((word)o, 16, 0);spc(1);rec_obj_print((word*)o);nl(1);}
  ret += _check_gc((word*)left);
  ret += _check_gc((word*)right);
  return ret;
}


word check_gc() {
  word * tr = (word*)((Object*)gc_set)->contents[0];
  return _check_gc(tr);
}


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
  }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type)){
    for (word i = 0; i < o->size; ++i){
      _mark_tc(heap, q, (word*)o->contents[i], cond);
    }
  }else if (!obj_cmp(type, array_type)){
    Object * arr = (Object*)o->contents[0];
    for (word i = 0; i < arr->size; ++i){
      _mark_tc(heap, q, (word*)arr->contents[i], cond);
    }
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


word sweep_tr(word * heap, word * tree) {
  word sweep(word * heap, word * obj);
  if (!tree){return 0;}
  AVL_Node * node = (AVL_Node*)tree;
  AVL_Node * left = (AVL_Node*)node->left;
  AVL_Node * right = (AVL_Node*)node->right;
  decrease_refcs((word*)node->data);
  word ret = sweep(heap, (word*)node->data);
  free(heap, (word*)node);
  ret += sweep_tr(heap, (word*)left);
  ret += sweep_tr(heap, (word*)right);
  return ret;
}


word sweep(word * heap, word * obj){
  Object * o = (Object*)obj;
  if (!o || !visited(obj)){return 0;}

  // Return if object has already been freed or is in the process of being freed
  word ** tr = (word**)&(((Object*)gc_set)->contents);
  AVL_Node * node = (AVL_Node*)avl_find(tr, (word)(obj+1), &avl_basic_cmp);
  if (!node){return 0;}

  word ret = 0;
  word * mem = set_remove(heap, gc_set, obj+1, &avl_basic_cmp);
  word * type = (word*)o->type;
  if (!obj_cmp(type, string_type) || !obj_cmp(type, num_type)){
    ret = 1;
  }else if (!obj_cmp(type, set_type)){
    ret += sweep_tr(heap, (word*)o->contents[0]);
  }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type)){
    for (word i = 0; i < o->size; ++i){
      decrease_refcs((word*)o->contents[i]);
      ret += sweep(heap, (word*)o->contents[i]);
    }
  }else if (!obj_cmp(type, array_type)){
    Object * arr = (Object*)o->contents[0];
    for (word i = 0; i < arr->size; ++i){
      decrease_refcs((word*)arr->contents[i]);
      ret += sweep(heap, (word*)arr->contents[i]);
    }
    void _object_delete(word * heap, word * arr);
    set_remove(heap, gc_set, (word*)arr + 1, &avl_basic_cmp);
    _object_delete(heap, (word*)arr);
  }
  object_delete(heap, type);
  free(heap, mem);
  --gc_num_alloced;
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
  if (!obj_cmp((word*)data->type, function_type)){
    queue_push(heap, q, (word)data);
  }
}


void gc_collect(word * heap, word * gc_set){
  word * tr = (word*)((Object*)gc_set)->contents[0];
  word * q = queue(heap);
  _gc_collect(heap, tr, q);
  word * data;
  while((data = (word*)queue_pop(heap, q))){
    //continue if obj has already been freed
    word ** tr = (word**)&(((Object*)gc_set)->contents);
    AVL_Node * node = (AVL_Node*)avl_find(tr, (word)(data+1), &avl_basic_cmp);
    if (!node){continue;}

    word * q = mark_tc(heap, data, 1);
    word * obj;
    while ((obj = (word*)queue_pop(heap, q))){
      if (get_tmp_rc(obj) < get_rc(obj)){
	mark_tc(heap, obj, 0);
      }
    }
    free(heap, q);

    sweep(heap, data);

    //check_gc();
    //print_uint(0xdeadbeef, 16, 0);
  }
  free(heap, q);
}


/* void cover_tree(word * tree, void (*fn)(word*)) { */
/*   if (!tree){return;} */
/*   AVL_Node * node = (AVL_Node*)tree; */
/*   AVL_Node * left = (AVL_Node*)node->left; */
/*   AVL_Node * right = (AVL_Node*)node->right; */
/*   fn((word*)node->data); */
/*   cover_tree((word*)left, fn); */
/*   cover_tree((word*)right, fn); */
/* } */


/* void decrease_refcs(word * obj){ */
/*   Object * o = (Object*)obj; */
/*   if (!o || visited(obj)){return;} */
/*   set_visited(obj, 1); */
/*   word * type = (word*)o->type; */
/*   word tmp_rc = get_tmp_rc(obj); */
/*   word rc = get_rc(obj); */
/*   if (rc <= tmp_rc){ */
/*     if (!obj_cmp(type, set_type)){ */
/*       cover_tree((word*)o->contents[0], &_decrease_refcs); */
/*     }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type)){ */
/*       for (word i = 0; i < o->size; ++i){ */
/* 	word * obj1 = (word*)o->contents[i]; */
/*         _decrease_refcs(obj1); */
/*       } */
/*     }else if (!obj_cmp(type, array_type)){ */
/*       Object * arr = (Object*)o->contents[0]; */
/*       for (word i = 0; i < arr->size; ++i){ */
/* 	word * obj1 = (word*)arr->contents[i]; */
/* 	_decrease_refcs(obj1); */
/*       } */
/*     } */
/*   } */

/*   if (!obj_cmp(type, set_type)){ */
/*     cover_tree((word*)o->contents[0], &decrease_refcs); */
/*   }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type)){ */
/*     for (word i = 0; i < o->size; ++i){ */
/*       decrease_refcs((word*)o->contents[i]); */
/*     } */
/*   }else if (!obj_cmp(type, array_type)){ */
/*     Object * arr = (Object*)o->contents[0]; */
/*     for (word i = 0; i < arr->size; ++i){ */
/*       decrease_refcs((word*)arr->contents[i]); */
/*     } */
/*   } */
/* } */


/* word sweep_tr(word * heap, word * tree) { */
/*   word sweep(word * heap, word * obj); */
/*   if (!tree){return 0;} */
/*   AVL_Node * node = (AVL_Node*)tree; */
/*   AVL_Node * left = (AVL_Node*)node->left; */
/*   AVL_Node * right = (AVL_Node*)node->right; */
/*   word ret = sweep(heap, (word*)node->data); */
/*   ret += sweep_tr(heap, (word*)left); */
/*   ret += sweep_tr(heap, (word*)right); */
/*   return ret; */
/* } */


/* word sweep(word * heap, word * obj){ */
/*   Object * o = (Object*)obj; */
/*   if (!o || !visited(obj)){return 0;} */

/*   // Return if object has already been freed */
/*   word ** tr = (word**)&(((Object*)gc_set)->contents); */
/*   AVL_Node * node = (AVL_Node*)avl_find(tr, (word)(obj+1), &avl_basic_cmp); */
/*   if (!node){return 0;} */
  
/*   word * type = (word*)o->type; */
/*   word ret = 0; */
/*   word tmp_rc = get_tmp_rc(obj); */
/*   word rc = get_rc(obj); */

/*   if (get_rc(obj) > tmp_rc){ */
/*     mark_tc(obj, 0); */
/*     return 0; */
/*   }else{ */
/*     word * mem = set_remove(heap, gc_set, obj+1, &avl_basic_cmp); */
/*     word * type = (word*)o->type; */
/*     if (!obj_cmp(type, string_type) || !obj_cmp(type, num_type)){ */
/*       ret = 1; */
/*     }else if (!obj_cmp(type, set_type)){ */
/*       ret += sweep_tr(heap, (word*)o->contents[0]); */
/*     }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type)){ */
/*       for (word i = 0; i < o->size; ++i){ */
/* 	ret += sweep(heap, (word*)o->contents[i]); */
/*       } */
/*     }else if (!obj_cmp(type, array_type)){ */
/*       Object * arr = (Object*)o->contents[0]; */
/*       for (word i = 0; i < arr->size; ++i){ */
/* 	ret += sweep(heap, (word*)arr->contents[i]); */
/*       } */
/*       void _object_delete(word * heap, word * arr); */
/*       set_remove(heap, gc_set, (word*)arr + 1, &avl_basic_cmp); */
/*       _object_delete(heap, (word*)arr); */
/*     } */
/*     object_delete(heap, type); */
/*     free(heap, mem); */
/*     --gc_num_alloced; */
/*     ret += 1; */
/*   } */
/* } */


/* word collect_obj(word * heap, word * obj) { */
/*   Object * o = (Object*)obj; */

/*   // Return if object has already been freed */
/*   word ** tr = (word**)&(((Object*)gc_set)->contents); */
/*   AVL_Node * node = (AVL_Node*)avl_find(tr, (word)(obj+1), &avl_basic_cmp); */
/*   if (!node){return 0;} */
/*   mark_tc(obj, 1); */
/*   if (get_tmp_rc(obj) < get_rc(obj)){ */
/*     mark_tc(obj, 0); */
/*     return 0; */
/*   } */
/*   decrease_refcs(obj); */
/*   print_uint(o->refcount, 16, 0); rec_obj_print(obj);spc(1); */
/*   return sweep(heap, obj); */
/* } */


/* void _gc_collect(word *heap, word *tree, word * q) { */
/*   if (!tree){return;} */
/*    AVL_Node * node = (AVL_Node*)tree; */
/*   AVL_Node * left = (AVL_Node*)node->left; */
/*   AVL_Node * right = (AVL_Node*)node->right; */
/*   _gc_collect(heap, (word*)left, q); */
/*   _gc_collect(heap, (word*)right, q); */
/*   Object * data = (Object*)((word*)node->data - 1); */
/*   if (!obj_cmp((word*)data->type, function_type)){ */
/*     queue_push(heap, q, (word)data); */
/*   } */
/* } */


/* void gc_collect(word * heap, word * gc_set){ */
/*   word * tr = (word*)((Object*)gc_set)->contents[0]; */
/*   word * q = queue(heap); */
/*   _gc_collect(heap, tr, q); */
/*   word * data; */
/*   while((data = (word*)queue_pop(heap, q))){ */
/*     collect_obj(heap, data); */
/*   } */
/*   free(heap, q); */
/* } */


/* //TODO: Make this not scan over the whole gc_set. */
/* word _gc_unmark(word * tree){ */
/*   if (!tree){return 0;} */
/*   AVL_Node * node = (AVL_Node*)tree; */
/*   AVL_Node * left = (AVL_Node*)node->left; */
/*   AVL_Node * right = (AVL_Node*)node->right; */
/*   word ret = 0; */
/*   Object * o = (Object*)((word*)node->data - 1); */
  
/*   if (o->refcount & seen){ */
/*     o->refcount &= refcount_mask; */
/*     ret = 1; */
/*   } */
/*   ret += _gc_unmark((word*)left); */
/*   ret += _gc_unmark((word*)right); */
/*   return ret; */
/* } */


/* word gc_unmark(word * gcs) { */
/*   word * tree = (word*)((Object*)gc_set)->contents[0]; */
/*   return _gc_unmark(tree); */
/* } */


/* word gc_check_tree(word * root, word * tree){ */
/*   if (!tree){return 0;} */
/*   word gc_check(word * root, word * obj); */
/*   AVL_Node * node = (AVL_Node*)tree; */
/*   AVL_Node * left = (AVL_Node*)node->left; */
/*   AVL_Node * right = (AVL_Node*)node->right; */
/*   word ret = gc_check(root, (word*)node->data); */
/*   ret += gc_check_tree(root, (word*)left); */
/*   ret += gc_check_tree(root, (word*)right); */
/*   return ret; */
/* } */


/* word gc_check(word * root, word * obj){ */
/*   if (!obj){ */
/*     return 0; */
/*   } */
/*   Object * o = (Object*)obj; */
/*   if (root == obj){ */
/*     o->refcount |= seen; */
/*     return 1; */
/*   } */
/*   if (o->refcount & seen){ */
/*     return 0; */
/*   } */
/*   o->refcount |= seen; */

/*   // Find how many times all things reachable from root reference root. */
/*   word * type = (word*)o->type; */
/*   word ret = 0; */
/*   if (!obj_cmp(type, set_type)){ */
/*     ret += gc_check_tree(root, (word*)o->contents[0]); */
/*   }else if (!obj_cmp(type, cell_type)){ */
/*     for (word i = 0; i < o->size; ++i){ */
/*       ret += gc_check(root, (word*)(o->contents[i])); */
/*     } */
/*   }else if(!obj_cmp(type, function_type)){ */
/*     ret += gc_check(root, (word*)(o->contents[0])); */
/*   }else if (!obj_cmp(type, array_type)){ */
/*     Object * arr = (Object*)o->contents[0]; */
/*     for (word i = 0; i < arr->size; ++i){ */
/*       ret += gc_check(root, (word*)(arr->contents[i])); */
/*     } */
/*   } */
/*   return ret; */
/* } */


/* word gc_check_fn(word * root){ */
/*   Object * r = (Object*)root; */
/*   word count = r->refcount & refcount_mask; */
/*   word res = gc_check_tree(root, (word*)((Object*)(r->contents[0]))->contents[0]); */
/*   return res; */
/* } */


/* word collect_obj_tr(word *heap, word * tree) { */
/*   if (!tree){return 0;} */
/*   AVL_Node * node = (AVL_Node*)tree; */
/*   AVL_Node * left = (AVL_Node*)node->left; */
/*   AVL_Node * right = (AVL_Node*)node->right; */
/*   word ret = collect_obj(heap, (word*)node->data); */
/*   ret += collect_obj_tr(heap, (word*)left); */
/*   ret += collect_obj_tr(heap, (word*)right); */
/*   return ret; */
/* } */


/* word collect_obj(word * heap, word *obj) { */
/*   Object * o = (Object*)obj; */
/*   word ret = 0; */
/*   if (!o){return 0;} */

/*   // Return if object has already been freed */
/*   word ** tr = (word**)&(((Object*)gc_set)->contents); */
/*   AVL_Node * node = (AVL_Node*)avl_find(tr, (word)(obj+1), &avl_basic_cmp); */
/*   if (!node){return 0;} */

/*   //Return if the tmp refcount has already been reset */
/*   word tmp_rc = get_tmp_rc(obj); */
/*   if (!tmp_rc){return 0;} */

/*   if (get_rc(obj) > tmp_rc){ */
/*     mark_tc(obj, 0); */
/*     return 0; */
/*   }else{ */
/*     word * mem = set_remove(heap, gc_set, obj+1, &avl_basic_cmp); */
/*     word * type = (word*)o->type; */
/*     if (!obj_cmp(type, string_type) || !obj_cmp(type, num_type)){ */
/*       ret = 1; */
/*     }else if (!obj_cmp(type, set_type)){ */
/*       ret += collect_obj_tr(heap, (word*)o->contents[0]); */
/*     }else if (!obj_cmp(type, cell_type) || !obj_cmp(type, function_type)){ */
/*       for (word i = 0; i < o->size; ++i){ */
/* 	ret += collect_obj(heap, (word*)o->contents[i]); */
/*       } */
/*     }else if (!obj_cmp(type, array_type)){ */
/*       Object * arr = (Object*)o->contents[0]; */
/*       for (word i = 0; i < arr->size; ++i){ */
/* 	ret += collect_obj(heap, (word*)arr->contents[i]); */
/*       } */
/*       void _object_delete(word * heap, word * arr); */
/*       _object_delete(heap, (word*)arr); */
/*     } */
/*     free(heap, mem); */
/*     --gc_num_alloced; */
/*     ret += 1; */
/*   } */
/*   return ret; */
/* } */
