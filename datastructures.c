#include "config.h"

//NOTE: Some of the datastructures here depend on the memory allocator leaving the size of the allocated memory just before the mem.

// Note: Use black magic to get maximum size from the umds associated with the object when expanding; No sense consuming another word.
// TODO: Refcount attributes, e.g. cyclic (also add in manual ability to deallocate or cycle check)
word set_keyfind_cmp(word * kvp, word * key);

word * num_type;
word * string_type;
word * array_type;
word * set_type;
word * function_type;
word * cell_type;
/* word * native_type; */
/* word * asm_type; */


const word obj_sz = sizeof(Object)/sizeof(word);

word * object(word * heap, word * type, word size, word * contents, word n_words){
  if (size < n_words){return (word*)0;}
  word * mem = gc_alloc(heap, size + obj_sz - 1);
  if (!mem){return (word*)0;}
  Object * obj = (Object*)(mem - 1);
  obj->refcount = 0;
  obj->type = (word)type;
  ((Object*)type)->refcount += 1;
  obj->size = n_words;
  if (contents){
    for (word i = 0; i < n_words; ++i){
      obj->contents[i] = contents[i];
    }
  }
  return (word*)obj;
}


word * object_append_word(word * heap, word * obj, word data){
  Object * o = (Object*)obj;
  word max_sz = o->max_sz - obj_sz;
  if (o->size >= max_sz){
    o = (Object*)(gc_realloc(heap, obj+1, obj_sz + max_sz + max_sz/2) - 1);
    if (!o){return 0;}
  }
  o->contents[o->size++] = (word)data;
  return (word*)o;
}


word * object_append(word * heap, word * obj, word * data){
  Object * o = (Object*)obj;
  Object * d = (Object*)data;
  ++d->refcount;
  word max_sz = o->max_sz - obj_sz;
  if (o->size >= max_sz){
    o = (Object*)(gc_realloc(heap, obj+1, obj_sz + max_sz + max_sz/2) - 1);
    if (!o){return 0;}
  }
  o->contents[o->size++] = (word)data;
  return (word*)o;
}


void obj_print(word * obj){
  Object * o = (Object*)obj;
  Object * type = (Object*)o->type;
  print_cstr("maxs: ");print_uint(o->max_sz, 16, 0);nl(1);
  print_cstr("refc: ");print_uint(o->refcount, 16, 0);nl(1);
  print_cstr("size: ");print_uint(o->size, 16, 0);nl(1);
  print_cstr("type: ");
  for (word i = 0; i < type->size * sizeof(word); ++i){
    print_cstr((char*)(type->contents) + i);
  }
  nl(1);print_cstr("cnts: ");
  for (word i = 0; i < o->size; ++i){
    print_uint(o->contents[i], 16, 0);spc(1);
  }
  nl(1);
}


void rec_obj_print(word * obj){
  Object * o = (Object*)obj;
  if (!o){
    print_cstr("Null ");
    return;
  }
  if (!obj_cmp((word*)o->type, string_type)){
    if (!o->size){
      print_cstr("''");
    }
    for (word i = 0; i < o->size*sizeof(word); ++i){
      print_cstr((char*)(o->contents) + i);
    }spc(1);
  }else if (!obj_cmp((word*)o->type, array_type)){
    Object * o1 = (Object*)o->contents[0];
    print_cstr("[");spc(1);
    for (word i = 0; i < o1->size; ++i){
      rec_obj_print((word*)o1->contents[i]);
    }
    print_cstr("]");spc(1);
  }else if (!obj_cmp((word*)o->type, num_type)){
    print_num(obj);spc(1);
  }else if (!obj_cmp((word*)o->type, set_type)){
    print_cstr("{");spc(1);
    print_set(obj);spc(1);
    print_cstr("}");spc(1);
  }else if (!obj_cmp((word*)o->type, cell_type)){
    print_cstr("(");spc(1);
    rec_obj_print((word*)o->contents[0]);
    rec_obj_print((word*)o->contents[1]);
    print_cstr(")");spc(1);
  }else if (!obj_cmp((word*)o->type, function_type)){
    //print_set((word*)o->contents[0]);
    print_cstr("<");spc(1);
    rec_obj_print((word*)o->contents[1]);
    rec_obj_print((word*)o->contents[2]);
    print_cstr(">");spc(1);
  }else{
    print_cstr("???");spc(1);
  }
}


void _object_delete(word * heap, word * obj){
  //rec_obj_print(obj);nl(1);
  gc_free(heap, obj + 1);
}

//TODO
void object_delete(word * heap, word * obj){
  Object *o = (Object *)obj;
  if (!o){return;}

  word * type = (word*)o->type;
  /* if (!obj_cmp(type, function_type)){ */
  /*   --(o->refcount); */
  /*   word n_freed = collect_obj(heap, obj); */
  /*   if (!n_freed){return;} */
  /* }else  */
  if (--(o->refcount)){
    return;
  }else{
    if (!obj_cmp(type, num_type) || !obj_cmp(type, string_type)){
      _object_delete(heap, obj);
    }else if(!obj_cmp(type, set_type)){
      set_delete(heap, obj);
    }else if(!obj_cmp(type, cell_type)){
      for (word i = 0; i < o->size; ++i){
	object_delete(heap, (word*)(o->contents[i]));
      }
      _object_delete(heap, obj);
    }else if (!obj_cmp(type, array_type)){
      obj_array_delete(heap, obj);
    }
  }
  object_delete(heap, type);
}


word * obj_array(word * heap, word size){
  word * arr = object(heap, (word*)0, size, (word*)0, 0);
  ++((Object*)arr)->refcount;
  word * ret = object(heap, array_type, 1, (word*)&arr, 1);
  return ret;
}


word obj_array_size(word * arr){
  Object * obj = (Object*)(((Object*)arr)->contents[0]);
  return obj->size;
}


void obj_array_append(word * heap, word * arr, word * data){
  Object * A = (Object*)arr;
  A->contents[0] = (word)object_append(heap, (word*)(A->contents[0]), data);
}


word * obj_array_idx(word * arr, word idx){
  Object * obj = (Object*)(((Object*)arr)->contents[0]);
  return (word*)(obj->contents[idx]);
}


void set_obj_array_idx(word * arr, word idx, word * val){
  Object * obj = (Object*)(((Object*)arr)->contents[0]);
  obj->contents[idx] = (word)val;
}


void obj_array_delete(word * heap, word * obj) {
  Object * o = (Object*)obj;
  Object * arr = (Object*)(o->contents[0]);
  _object_delete(heap, obj);
  if (--arr->refcount){
    return;
  }
  for (word i = 0; i < arr->size; ++i){
    object_delete(heap, (word*)(arr->contents[i]));
  }
  _object_delete(heap, (word*)arr);
}


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


word * array_append_str(word * heap, word * arr, uint8_t * str){
  for (word i = 0; str[i]; ++i){
    word x = str[i];
    arr = array_append(heap, arr, &x);
    if (!arr){return (word*)0;}
  }
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


AVL_Node * get_parent(AVL_Node * node){
  if (!node){return 0;}
  return (AVL_Node *)(node->prev & ((word)-1 << 2));
}


word get_balance_factor(word * nd){
  if (!nd){return 3;}
  return ((AVL_Node*)nd)->prev & 3;
}


word balance_factor(word * nd){
  if (!nd){return 0;}
  word bf = get_balance_factor(nd);
  //if (!bf){return 0;}
  //3 -> 0, 1 -> +1, 2 -> -1
  return (bf/3) ? 0 : nat_pow(-1, (bf % 3) - 1);
}


word rev_balance_factor(word bf){
  //-1 -> 2, 0 -> 3, 1 -> 1
  return bf ? (bf + 1 ? 1 : 2) : 3;
}


word set_balance_factor(word * nd, word bf){
  AVL_Node * node = (AVL_Node*)nd;
  if (!nd){return 1;}
  node->prev &= (word)-1 << 2;
  node->prev |= rev_balance_factor(bf);
  return 0;
}


void avl_move(word ** tr, word * dest, word * src){
  if (dest == src){return;}
  AVL_Node * s = (AVL_Node*)src;
  AVL_Node * d = (AVL_Node*)dest;

  AVL_Node * parent = get_parent(s);
  if (parent){
    if ((word*)parent->left == src){
      parent->left = (word)dest;
    }else{
      parent->right = (word)dest;
    }
  }
  AVL_Node * left = (AVL_Node*)(s->left);
  if (left){
    left->prev = (word)(dest) | get_balance_factor((word*)left);
  }
  AVL_Node * right = (AVL_Node*)(s->right);
  if (right){
    right->prev = (word)(dest) | get_balance_factor((word*)right);
  }

  d->prev = s->prev;
  d->left = (word)left;
  d->right = (word)right;
  d->data = s->data;
  
  if (*tr == src){
    *tr = dest;
  }
}


void avl_merge(word ** tr, word * addr, word size){
  word * amax = addr + size;
  AVL_Node * left = (AVL_Node*)0;
  AVL_Node * right = (AVL_Node*)0;
  AVL_Node * tree = (AVL_Node*)*tr;
  while (tree && (!left || !right)){
    if ((word*)tree == amax){
      right = (AVL_Node*)tree;
      tree = (AVL_Node*)(tree->left);
    }else if ((word*)tree + tree->data == addr){
      left = (AVL_Node*)tree;
      tree = (AVL_Node*)(tree->right);
    }

    if (tree){
      if ((word*)tree > amax){
  	tree = (AVL_Node*)(tree->left);
      }else if ((word*)tree + tree->data < addr){
  	tree = (AVL_Node*)(tree->right);
      }
    }
  }

  if (right){
    size += right->data;
    word * dest = _avl_delete(tr, (word*)right, &avl_mem_cmp);
    avl_move(tr, dest, (word*)right);
  }
  if (left){
    addr -= left->data;
    size += left->data;
    word * dest = _avl_delete(tr, (word*)left, &avl_mem_cmp);
    avl_move(tr, dest, (word*)left);
  }
  _avl_insert(tr, addr, size, &avl_mem_cmp);
}


word * avl_find(word ** tr, word data, word (*cmp)(word*, word*)){
  AVL_Node * tree = (AVL_Node*)*tr;
  AVL_Node node = {0, 0, 0, data};
  for (word b = cmp((word*)tree, (word*)&node); b; b = cmp((word*)tree, (word*)&node)){
    switch (b){
    case -1:
      tree = (AVL_Node*)(tree->left);
    case 0:
      break;
    case 1:
      tree = (AVL_Node*)(tree->right);
    }
    if (!tree){return (word*)0;}
  }
  return (word*)tree;
}


word * avl_min_ge(word * tree, word data){
  if (!tree){return 0;}
  word * o1 = avl_min_ge((word*)((AVL_Node*)tree)->left, data);
  word * o2 = 0;
  if (!o1){
    o2 = avl_min_ge((word*)((AVL_Node*)tree)->right, data);
  }

  if (o1){
    return (word*)o1;
  }else if (o2){
    return (word*)o2;    
  }else if (((AVL_Node*)tree)->data >= data){
    return tree;
  }
  
  return (word*)0;
}


word * avl_rotate_left(word ** tr, word * nd){
  AVL_Node * node = (AVL_Node*)nd; 
  if (!node){return 0;} 
  AVL_Node * parent = get_parent(node);
  AVL_Node * root = (AVL_Node*)(node->right);
  word old_rootbf = balance_factor((word*)root);
  if (!root){return 0;}
  AVL_Node * rl_child = (AVL_Node*)(root->left);

  node->right = root->left;
  node->prev = (word)root;

  root->left = (word)node;
  root->prev = (word)parent;

  if (rl_child){
    rl_child->prev = (word)node | get_balance_factor((word*)rl_child);
  }

  if (parent){
    if (parent->left == (word)node){
      parent->left = (word)root;
    }else{
      parent->right = (word)root;
    }
  }else{
    *tr = (word*)root;
  }

  if (old_rootbf){
    node->prev |= rev_balance_factor(0);
    root->prev |= rev_balance_factor(0);
  } else {
    node->prev |= rev_balance_factor(1);
    root->prev |= rev_balance_factor(-1);
  }

  return (word*)root;
}


word * avl_rotate_right(word ** tr, word * nd){
  AVL_Node * node = (AVL_Node*)nd; 
  if (!node){return 0;} 
  AVL_Node * parent = get_parent(node);
  AVL_Node * root = (AVL_Node*)(node->left);
  word old_rootbf = balance_factor((word*)root);
  if (!root){return 0;}
  AVL_Node * lr_child = (AVL_Node*)(root->right);

  node->left = root->right;
  node->prev = (word)root;

  root->right = (word)node;
  root->prev = (word)parent;

  if (lr_child){
    lr_child->prev = (word)node | get_balance_factor((word*)lr_child);
  }

  if (parent){
    if (parent->left == (word)node){
      parent->left = (word)root;
    }else{
      parent->right = (word)root;
    }
  }else{
    *tr = (word*)root;
  }

  if (old_rootbf){
    node->prev |= rev_balance_factor(0);
    root->prev |= rev_balance_factor(0);
  } else {
    node->prev |= rev_balance_factor(-1);
    root->prev |= rev_balance_factor(1);
  }

  return (word*)root;
}


word * avl_rotate_lr(word ** tr, word * node){
  AVL_Node * child = (AVL_Node*)(((AVL_Node*)node)->left);
  AVL_Node * lr_child = (AVL_Node*)(child->right);
  word bf = balance_factor((word*)lr_child);
  avl_rotate_left(tr, (word*)child);
  word * ret = avl_rotate_right(tr, (word*)node);
  if (bf == 1) {
    set_balance_factor((word*)child, (word)-1);
    set_balance_factor((word*)node, 0);
  }else if (bf == (word)-1){
    set_balance_factor((word*)node, 1);
    set_balance_factor((word*)child, 0);
  }else{
    set_balance_factor((word*)child, 0);
    set_balance_factor((word*)node, 0);
  }
  word * pp = (word*)get_parent((AVL_Node*)node);
  set_balance_factor(pp, 0);
  return ret;
}


word * avl_rotate_rl(word ** tr, word * node){
  AVL_Node * child = (AVL_Node*)(((AVL_Node*)node)->right);
  AVL_Node * rl_child = (AVL_Node*)(child->left);
  word bf = balance_factor((word*)rl_child);
  avl_rotate_right(tr, (word*)child);
  word * ret = avl_rotate_left(tr, (word*)node);
  if (bf == 1) {
    set_balance_factor((word*)node, (word)-1);
    set_balance_factor((word*)child, 0);
  }else if (bf == (word)-1){
    set_balance_factor((word*)child, 1);
    set_balance_factor((word*)node, 0);
  }else{
    set_balance_factor((word*)child, 0);
    set_balance_factor((word*)node, 0);
  }
  word * pp = (word*)get_parent((AVL_Node*)node);
  set_balance_factor(pp, 0);
  return ret;
}


word avl_tree_height(word * nd){
  AVL_Node * node = (AVL_Node *)nd;
  if (!node){return -1;}
  return 1 + max((sword)avl_tree_height((word*)(node->left)), (sword)avl_tree_height((word*)(node->right)));
}


// cmp must return one of -1, 0, 1.
word _avl_insert(word ** tr, word * nd, word data, word (*cmp)(word*, word*)){
  AVL_Node * tree = (AVL_Node*)(*tr);
  AVL_Node * node = (AVL_Node*)nd;
  //print_uint(node, 16, 0);spc(1);print_uint(data, 16, 0);nl(1);
  if (!node){
    return 1;
  }

  node->prev = 3;
  node->left = 0;
  node->right = 0;
  node->data = data;

  if (!tree){
    *tr = (word*)node;
    return 0;
  }

  // Insert node
  AVL_Node * ins_p = 0;
  word b = 0;
  while(tree){
    b = cmp((word*)tree, (word*)node);
    switch (b){
    case -1:
      ins_p = tree;
      tree = (AVL_Node*)tree->left;
      break;
    case 0:
      return 1;
    case 1:
      ins_p = tree;
      tree = (AVL_Node*)tree->right;
      break;
    }
  }
  node->prev = (word)ins_p | 3;
  switch(b){
  case -1:
    ins_p->left = (word)node;
    break;
  case 1:
    ins_p->right = (word)node;
    break;
  }

  // If the height of the tree did not increase, return.
  AVL_Node * node_par = get_parent(node);
  if (node_par->left && node_par->right){
    node_par->prev |= 3;
    return 0;
  }

  // Fix tree if necessary.
  AVL_Node * child = node;
  for (node = get_parent(node); node; node = get_parent(node)){
    word bf = 0;
    if ((word)child == node->left){
      bf = (word)-1 + balance_factor((word*)node);
    }else{
      bf = 1 + balance_factor((word*)node);
    }
    if (bf == (word)-2){
      if (balance_factor((word*)(node->left)) == (word)-1){
	avl_rotate_right(tr, (word*)node);
      }else{
	avl_rotate_lr(tr, (word*)node);
      }
      break;
    }else if (bf == (word)2){
      if (balance_factor((word*)(node->right)) == 1){
	avl_rotate_left(tr, (word*)node);
      }else{
	avl_rotate_rl(tr, (word*)node);
      }
      break;
    }else if (!bf){
      set_balance_factor((word*)node, bf);
      return 0;
    }else{
      set_balance_factor((word*)node, bf);
    }
    child = node;
  }
  return 0;
}


word avl_insert(word * heap, word ** tr, word data, word (*cmp)(word*, word*)){
  AVL_Node ** tree = (AVL_Node**)tr;
  word * node = alloc(heap, sizeof(AVL_Node)/sizeof(word));
  if (!node){
    return 1;
  }

  word ret = _avl_insert(tr, node, data, cmp);
  if (ret){
    free(heap, (word*)node);
  }
  return ret;
}


word * _avl_delete(word ** tr, word * node, word (*cmp)(word*, word*)){
  word * ret = 0;
  AVL_Node * tree = (AVL_Node*)node;
  if (!(*tr) || !tree){return ret;}

  // Find successor (if it exists)
  AVL_Node * successor = (AVL_Node*)(tree->left);
  if (tree->right){
    successor = (AVL_Node*)(tree->right);
    while (successor->left){
      successor = (AVL_Node*)successor->left;
    }
  }

  AVL_Node * child = 0;
  word bf = 0;
  word direction = 0;
  // Will start rebalancing tree from parent later.
  AVL_Node * parent;
  if (successor){
    parent = get_parent(successor);
    // successor must have a parent if it exists. (could be tree)
    tree->data = successor->data;
    child = (AVL_Node*)(successor->right);
    word cbf = get_balance_factor((word*)child);
    if (child){((AVL_Node*)child)->prev = (word)parent | cbf;}
    if (parent->left == (word)successor){
      parent->left = (word)child;
      direction = (word)-1;
    }else{
      parent->right = (word)child;
      direction = 1;
    }
    ret = (word*)successor;
  }else{
    parent = get_parent(tree);
    if (!parent){
      *tr = (word*)0;
      ret = (word*)tree;
      return ret;
    }
    // tree must have a parent at this point, and tree must have no children. (tree is a leaf node)
    if (parent->left == (word)tree){
      parent->left = 0;
      direction = (word)-1;
    }else if (parent->right == (word)tree){
      parent->right = 0;
      direction = 1;
    }
    ret = (word*)tree;
  }

  AVL_Node * pp = 0;
  for (; parent; parent = pp){
    pp = get_parent(parent);
    word bf = balance_factor((word*)parent) - direction;
    word _bf = 1; // Just has to not be zero.
    // Set direction for next iteration
    if (pp){
      if (pp->left == (word)parent){
	direction = (word)-1;
      }else{
	direction = 1;
      }
    }
    switch(bf){
    case -2:
      AVL_Node * l_child = (AVL_Node*)parent->left;
      _bf = balance_factor((word*)l_child);
      if (_bf == 1){
        avl_rotate_lr(tr, (word*)parent);
      }else{
        avl_rotate_right(tr, (word*)parent);
      }
      break;
    case -1:
      set_balance_factor((word*)parent, bf);
      return ret;
    case 0:
      set_balance_factor((word*)parent, bf);
      break;
    case 1:
      set_balance_factor((word*)parent, bf);
      return ret;
    case 2:
      AVL_Node * r_child = (AVL_Node*)parent->right;
      _bf = balance_factor((word*)r_child);
      if (_bf == (word)-1){
        avl_rotate_rl(tr, (word*)parent);
      }else{
        avl_rotate_left(tr, (word*)parent);
      }
      break;
    }
    if (!_bf){return ret;}
  }

  return ret;
}


word avl_delete(word * heap, word ** tr, word data, word (*cmp)(word*, word*)){
  word * node = avl_find(tr, data, cmp);
  if (!node){return 1;}
  word * x = _avl_delete(tr, node, cmp);
  if (x){
    free(heap, x);
    return 0;
  }
  return 1;
}


word avl_basic_cmp(word * n1, word * n2){
  AVL_Node * node1 = (AVL_Node*)n1;
  AVL_Node * node2 = (AVL_Node*)n2;

  if (node1->data < node2->data){
    return 1;
  }else if (node1->data == node2->data){
    return 0;
  }
  return (word)-1;
}


word avl_mem_cmp(word * n1, word * n2){
  if (n1 < n2){
    return 1;
  }else if (n1 == n2){
    return 0;
  }
  return (word)-1;
}


void _print_avl(word * tree, word space, word inc){
  AVL_Node * node = (AVL_Node*)tree;
  if (!tree){return;}
  spc(space);print_uint(tree[0] & 3, 16, 2);//nl(1);
  spc(1);print_uint(tree[3], 16, 2);nl(1);
  //spc(1);print_uint(tree, 16, 8);nl(1);
  _print_avl((word*)(node->right), space + inc, inc);
  _print_avl((word*)(node->left), space, inc);
}


void print_avl(word * tree, word space, word inc){
  if (!tree){print_uint(0, 16, 2);nl(1);}
  _print_avl(tree, space, inc);
}


word * check_balance_factors(word * tr){
  AVL_Node * tree = (AVL_Node*)tr;
  if (!tree){return 0;}
  if (!tree->left && !tree->right){return 0;}
  word lh = avl_tree_height((word*)(tree->left));
  word rh = avl_tree_height((word*)(tree->right));
  word bf = balance_factor((word*)tree);

  word * lerr = check_balance_factors((word*)(tree->left));
  if (lerr){
    return lerr;
  }
  word * rerr = check_balance_factors((word*)(tree->right));
  if (rerr){
    return rerr;
  }

  switch (bf){
  case -1:
    if (!(lh == rh + 1)){return (word*)tree;}
    break;
  case 0:
    if (!(lh == rh)){return (word*)tree;}
    break;
  case 1:
    if (!(lh + 1 == rh)){return (word*)tree;}
  }
  return 0;
}


void init_types(void){
  word str_str[3] = {'s', 't', 'r'};
  string_type = object(global_heap_start , (word*)0, 3, str_str, 3);
  ((Object *)string_type)->type = (word)string_type;
  ((Object *)string_type)->refcount += 2;
  word num_str[3] = {'n', 'u', 'm'};
  num_type = object(global_heap_start, string_type, 3, num_str, 3);
  ++((Object *)num_type)->refcount;
  word arr_str[3] = {'a', 'r', 'r'};
  array_type = object(global_heap_start, string_type, 3, arr_str, 3);
  ++((Object *)array_type)->refcount;
  word set_str[3] = {'s', 'e', 't'};
  set_type = object(global_heap_start, string_type, 3, set_str, 3);
  ++((Object *)set_type)->refcount;
  word fun_str[3] = {'f', 'u', 'n'};
  function_type = object(global_heap_start, string_type, 3, fun_str, 3);
  ++((Object *)function_type)->refcount;
  word cel_str[3] = {'c', 'e', 'l'};
  cell_type = object(global_heap_start, string_type, 3, cel_str, 3);
  ++((Object *)cell_type)->refcount;
  /* word native_str[3] = {'n', 't', 'v'}; */
  /* native_type = object(global_heap_start, string_type, 3, native_str, 3); */
  /* word asm_str[3] = {'a', 's', 'm'}; */
  /* asm_type = object(global_heap_start, string_type, 3, asm_str, 3); */
}


//NOTE: From this point forward only use if types are initialized.

word * pair(word * heap, word * obj1, word * obj2){
  word contents[2] = {(word)obj1, (word)obj2};
  Object * o1 = (Object *)obj1;
  Object * o2 = (Object *)obj2;
  o1->refcount += 1;
  o2->refcount += 1;
  return object(heap, cell_type, 2, contents, 2);
}


word obj_cmp(word * obj1, word * obj2){
  Object * p1 = (Object*)obj1;
  Object * p2 = (Object*)obj2;
  if (p1->size < p2->size){
    return 1;
  }else if (p1->size > p2->size){
    return (word)-1;
  }
  for (word i = 0; i < p1->size; ++i){
    if (p1->contents[i] < p2->contents[i]){
      return 1;
    }else if (p1->contents[i] > p2->contents[i]){
      return (word)-1;
    } 
  }
  return 0;
}


word set_cmp(word * node1, word * node2){
  Object * p1 = (Object*)(((AVL_Node*)node1)->data);
  Object * p2 = (Object*)(((AVL_Node*)node2)->data);
  word val = obj_cmp((word*)p1, (word*)p2);
  return val;
}


word set_keycmp(word * pair1, word * pair2){
  Object * p1 = (Object*)(((AVL_Node*)pair1)->data);
  Object * p2 = (Object*)(((AVL_Node*)pair2)->data);
  word * k1 = (word*)p1->contents[0];
  word * k2 = (word*)p2->contents[0];
  word val = obj_cmp(k1, k2);
  return val;
}


word set_keyfind_cmp(word * kvp, word * key){
  Object * pair = (Object*)(((AVL_Node*)kvp)->data);
  word * k1 = (word*)pair->contents[0];
  word * k2 = (word*)(((AVL_Node*)key)->data);
  word val = obj_cmp(k1, k2);
  return val;
}


word * set(word * heap){
  word contents = 0;
  return object(heap, set_type, 1, &contents, 1);
}


word set_add(word * heap, word * s, word * data, word (*cmp)(word*, word*)){
  Object * obj = (Object*)s;
  Object * d_obj = (Object*)data;
  if (data){d_obj->refcount += 1;}
  return avl_insert(heap, (word**)&(obj->contents), (word)data, cmp);
}


word * set_remove(word * heap, word * s, word * data, word (*cmp)(word*, word*)){
  word ** tr = (word**)&(((Object*)s)->contents);
  AVL_Node * node = (AVL_Node*)avl_find(tr, (word)data, cmp);
  if (!node){return 0;}
  word * res = (word*)node->data;
  word * x = _avl_delete(tr, (word*)node, &avl_basic_cmp);
  if (x){
    free(heap, x);
    return res;
  }
  return 0;
}

word set_add_str_key(word * heap, word * s, word * key, word * val){
  word * data = pair(heap, key, val);
  return set_add(heap, s, data, &set_keycmp);
}


word set_set_key(word * s, word * key, word * val){
  word ** tr = (word**)&(((Object*)s)->contents);
  AVL_Node * node = (AVL_Node*)avl_find(tr, (word)key, &set_keyfind_cmp);
  if (!node){return 0;}
  Object * pear = (Object*)node->data;
  word old_val = pear->contents[1];
  pear->contents[1] = (word)val;
  return old_val;
}


/* word set_remove_str_key(word * heap, word * set, word * key){ */
/*   word ** tr = (word**)&(((Object*)set)->contents); */
/*   AVL_Node * node = (AVL_Node*)avl_find(tr, (word)key, &set_keyfind_cmp); */
/*   if (!node){return 0;} */
/*   object_delete(heap, (word*)node->data); */
/*   word * x = _avl_delete(tr, (word*)node, &avl_basic_cmp); */
/*   if (x){ */
/*     free(heap, x); */
/*     return 0; */
/*   } */
/*   return 1; */
/* } */


word * set_remove_str_key(word * heap, word * set, word * key){
  word ** tr = (word**)&(((Object*)set)->contents);
  AVL_Node * node = (AVL_Node*)avl_find(tr, (word)key, &set_keyfind_cmp);
  if (!node){return 0;}
  word * res = (word*)node->data;
  word * x = _avl_delete(tr, (word*)node, &avl_basic_cmp);
  if (x){
    free(heap, x);
    return res;
  }
  return 0;
}


word * in_set(word * set, word * obj){
  word ** tr = (word**)&(((Object*)set)->contents);
  AVL_Node * node = (AVL_Node*)avl_find(tr, (word)obj, &set_cmp);
  if (!node){return 0;}
  return (word*)(node->data);
}


word * set_get_value(word * set, word * obj){
  word ** tr = (word**)&(((Object*)set)->contents);
  AVL_Node * node = (AVL_Node*)avl_find(tr, (word)obj, &set_keyfind_cmp);
  if (!node){return 0;}
  Object * pair = (Object*)(node->data);
  word * val = (word*)(pair->contents[1]);
  return val;
}


void _print_set(word * tree){
  if (!tree){return;}
  AVL_Node * node = (AVL_Node*)tree;
  rec_obj_print((word*)node->data);
  _print_set((word*)(node->right));
  _print_set((word*)(node->left));
}


void print_set(word * s){
  word * tree = (word*)((Object*)s)->contents[0];
  _print_set(tree);
}


void set_delete(word * heap, word * set){
  word ** tr = (word**)&(((Object*)set)->contents);
  while (*tr){
    AVL_Node * root = (AVL_Node*)*tr;
    object_delete(heap, (word*)(root->data));
    avl_delete(heap, tr, root->data, avl_basic_cmp);
  }
  _object_delete(heap, set);
}


word * queue(word * heap){
  Queue * q = (Queue*)alloc(heap, sizeof(Queue)/sizeof(word));
  if (!q){return (word*)q;}
  q->first = 0;
  q->last = 0;
  return (word*)q;
}


word * queue_push(word * heap, word * queue, word data){
  Link * link = (Link*)alloc(heap, sizeof(Link)/sizeof(word));
  if (!link){return (word*)link;}
  Queue * q = (Queue*)queue;
  link->data = data;
  link->next = (word)(q->first);
  link->prev = 0;
  if (!q->first){q->last = (word)link;}
  else{((Link*)(q->first))->prev = (word)link;}
  q->first = (word)link;
  return (word*)link;
}


word queue_last(word * heap, word * queue){
  Queue * q = (Queue*)queue;
  Link * last = (Link*)(q->last);
  if (!last){return 0;}
  q->last = last->prev;
  if (!q->last){q->first = 0;}
  else{((Link*)(last->prev))->next = 0;}
  word data = last->data;
  free(heap, (word*)last);
  return data;
}


word queue_pop(word * heap, word * queue){
  Queue * q = (Queue*)queue;
  Link * first = (Link*)(q->first);
  if (!first){return 0;}
  q->first = first->next;
  if (!q->first){q->last = 0;}
  else{((Link*)(first->next))->prev = 0;}
  word data = first->data;
  free(heap, (word*)first);
  return data;
}


/* word b16_to_word(word * num, word length){ */
/*   word neg = 0; */
/*   if (length){ */
/*     if (num[0] == '-'){ */
/*       neg = 1; */
/*       ++num; */
/*       --length; */
/*     } */
/*     if (!length || length > sizeof(word)*2){return 0;} */
/*   }else{ */
/*     return 0; */
/*   } */

/*   word final_num = 0; */
/*   word offset = '0'; */
/*   for (word index = 0; index < length; ++index){  */
/*     word chr = num[index]; */
/*     if ('f' >= chr && chr >= 'a') {offset = 'a'-10;} */
/*     else if ('F' >= chr && chr >= 'A'){offset = 'A'-10;} */
/*     else if('9' >= chr && chr >= '0') {offset = '0';} */
/*     else {return 0;} */
/*     final_num |= ((chr - offset) << (4*(length-index-1))); */
/*   } */

/*   if (neg){ */
/*     final_num  = ~final_num + 1; */
/*   } */

/*   return final_num; */
/* } */


word adc(word x, word y, word * result){
  *result = x + y;
  word mask = (word)-1 >> 1;
  word z = (x & mask) + (y & mask);
  z >>= sizeof(word)*8 - 1;
  x >>= sizeof(word)*8 - 1;
  y >>= sizeof(word)*8 - 1;
  return (x & y) | (x & z) | (y & z);
}


void num_negate(word * num){
  Object * n = (Object*)num;
  word carry = 1;
  for (word i = 0; i < n->size; ++i){
    carry = adc(~(n->contents[i]), carry, n->contents + i);
  }
}


word * num_add(word * heap, word * num1, word * num2){
  Object * n1 = (Object*)num1;
  Object * n2 = (Object*)num2;

  // Put the "shorter" number last. 
  if (n1->size < n2->size){
    Object * tmp = n1;
    n1 = n2;
    n2 = tmp;
  }
  word n1_sz = n1->size;
  word n2_sz = n2->size;
  word n1_sgn = n1->contents[n1_sz-1] >> (sizeof(word)*8 - 1);
  word n2_sgn = n2->contents[n2_sz-1] >> (sizeof(word)*8 - 1);

  // Create an obj of size n1_sz because n1 is now the "longest" number.
  Object * result = (Object*)object(heap, num_type, n1_sz, 0, 0);
  result->size = n1_sz;
  word carry = 0;
  for (word i = 0; i < n2_sz; ++i){
    // Can add to the carry because only one of these two adds will overflow.
    carry = adc(n1->contents[i], carry, result->contents + i);
    carry += adc(result->contents[i], n2->contents[i], result->contents + i);
  }

  word padding = 0;
  if (n2_sgn){
    padding = (word)-1;
  }
  for (word i = n2_sz; i < n1_sz; ++i){
    carry = adc(n1->contents[i], carry, result->contents + i);
    carry += adc(result->contents[i], padding, result->contents + i);
  }

  // Always maintain a sign word at the end of the bignum
  if (n1_sgn && n2_sgn && (result->contents[n1_sz-1] != (word)-1)){
    result = (Object*)object_append_word(heap, (word*)result, (word)-1);
  }else if (!n1_sgn && !n2_sgn && result->contents[n1_sz-1]){
    result = (Object*)object_append_word(heap, (word*)result, 0);
  }else{
    word last = result->contents[n1_sz-1];
    for (word i = n1_sz-1; i > 2; --i){
      if (result->contents[i-1] == last){
  	--result->size;
      }
    }
  }

  return (word*)result;
}


word * str_to_num(word * heap, word * num){
  word neg = 0;
  word length = ((Object*)num)->size;
  word * arr = ((Object*)num)->contents;

  if (length){
    if (arr[0] == '-'){
      neg = 1;
      ++arr;
      --length;
    }
    if (!length){return 0;}
  }else{
    return 0;
  }

  word size = (length / (sizeof(word) * 2));
  Object * final_num = (Object*)object(heap, num_type, size, 0, 0);
  word offset = '0';
  word current = 0;

  word os = 0;
  word c = 0;
  for (; os < length % (sizeof(word) * 2); ++os){
    word chr = arr[os];
    if ('f' >= chr && chr >= 'a') {offset = 'a'-10;}
    else if ('F' >= chr && chr >= 'A'){offset = 'A'-10;}
    else if ('9' >= chr && chr >= '0'){offset = '0';}
    else {_object_delete(heap, (word*)final_num); return 0;}
    c <<= 4;
    c |= (chr - offset);
  }

  for (word i = 0; i < size; ++i){
    word j;
    for (word index = os; index < (sizeof(word) * 2) + os && (j = (size - i - 1) * sizeof(word)*2 + index) < length; ++index){
      word chr = arr[j];
      if ('f' >= chr && chr >= 'a') {offset = 'a'-10;}
      else if ('F' >= chr && chr >= 'A'){offset = 'A'-10;}
      else if ('9' >= chr && chr >= '0'){offset = '0';}
      else {_object_delete(heap, (word*)final_num); return 0;}
      current <<= 4;
      current |= (chr - offset);
    }
    final_num = (Object*)object_append_word(heap, (word*)final_num, current);
    current = 0;
  }

  //Add a word if final_num == 0 and is of size 0
  if (c || !(size || c)){
    final_num = (Object*)object_append_word(heap, (word*)final_num, c);
  }
  final_num = (Object*)object_append_word(heap, (word*)final_num, 0);

  if (neg){
    num_negate((word*)final_num);
  }

  return (word*)final_num;
}


void print_num(word * num){
  Object * n = (Object*)num;
  if (n->size == 1){
    print_uint(n->contents[0], 16, 0);
  }else if (n->size > 1){
    word last = n->contents[n->size - 1];
    if (last){
      print_cstr("-");
      num_negate(num);
    }
    print_uint(n->contents[n->size - 2], 16, 0);
    for (word i = n->size - 2; i > 0; --i){
      print_uint(n->contents[i-1], 16, sizeof(word)*2);
    }
    if (last){
      num_negate(num);
    }
  }
}
