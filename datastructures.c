#include "config.h"

//NOTE: Some of the datastructures here depend on the memory allocator leaving the size of the allocated memory just before the mem.

word set_keyfind_cmp(word * kvp, word * key);

word * num_type;
word * string_type;
word * array_type;
word * subarray_type;
word * set_type;
word * function_type;
word * cell_type;
word * bcode_type;
/* word * native_type; */


const word obj_sz = sizeof(Object)/sizeof(word);

word * object(word * heap, word * type, word size, word * contents, word n_words){
  Object * obj = (Object*)0;
  if (size < n_words){return (word*)0;}

  word * mem = alloc(heap, size + obj_sz - 1);
  if (!mem){return (word*)0;}
  obj = (Object*)(mem - 1);
  obj->refcount = 0;
  obj->type = (word)type;
  if (type){((Object*)type)->refcount += 1;}
  obj->size = n_words;
  if (contents){
    for (word i = 0; i < n_words; ++i){
      obj->contents[i] = contents[i];
    }
  }
  return (word*)obj;
}


word * object_append_word(word * heap, word * obj, word data){
  Object * o = 0;
  if (!obj){return 0;}
  o = (Object*)obj;
  word max_sz = o->max_sz - obj_sz;
  //print_cstr("Before: ");print_uint(o, 16, 0);nl(1);obj_print(o);nl(1);
  if (o->size >= max_sz){
    o = (Object*)(realloc(heap, obj+1, obj_sz + o->size + o->size/2 + 1) - 1);
    //if ((word*)o == (word*)0x410d08){while(1);}
    if (!((word*)o+1)){return 0;}
  }
  o->contents[o->size] = (word)data;
  ++o->size;
  //print_cstr("After:\n");obj_print(o);nl(1);
  return (word*)o;
}


word * object_append(word * heap, word * obj, word * data){
  Object * d = 0;
  if (!obj || !data){return 0;}
  d = (Object*)data;
  ++d->refcount;
  return (word*)object_append_word(heap, obj, (word)data);
}


word * object_expand(word * heap, word * obj, word new_sz){
  word old_sz = ((Object*)obj)->size;
  word * new = realloc(heap, obj+1, obj_sz + new_sz) - 1;
  if (new && new_sz < old_sz){((Object*)new)->size = new_sz;}
  return new;
}


void obj_print(word * obj){
  Object * o = (Object*)obj;
  Object * type = (Object*)o->type;
  uart_puts("maxs: ");uart_print_uint(o->max_sz, 16);uart_puts("\n");
  uart_puts("refc: ");uart_print_uint(o->refcount, 16);uart_puts("\n");
  uart_puts("size: ");uart_print_uint(o->size, 16);uart_puts("\n");
  uart_puts("type: ");
  for (word i = 0; i < type->size * sizeof(word); ++i){
    uart_puts((char*)(type->contents) + i);
  }
  uart_puts("\n");uart_puts("cnts: ");
  for (word i = 0; i < o->size; ++i){
    uart_print_uint(o->contents[i], 16);uart_puts(" ");
  }
  uart_puts("\n");
}


void rec_obj_print(word * obj){
  Object * o = (Object*)obj;
  if (!o){
    uart_puts("Null ");
    return;
  }
  if (!o->type){
    uart_puts("No Type ");
    return;
  }else if (!obj_cmp((word*)o->type, string_type)){
    if (!o->size){
      uart_puts("''");
    }
    for (word i = 0; i < o->size*sizeof(word); ++i){
      uart_puts((char*)(o->contents) + i);
    }uart_puts(" ");
  }else if (!obj_cmp((word*)o->type, array_type)){
    Object * o1 = (Object*)o->contents[0];
    uart_puts("[");uart_puts(" ");
    for (word i = 0; i < o1->size; ++i){
      rec_obj_print((word*)o1->contents[i]);
    }
    uart_puts("]");uart_puts(" ");
  }else if (!obj_cmp((word*)o->type, num_type)){
    print_num(obj);uart_puts(" ");
  }else if (!obj_cmp((word*)o->type, set_type)){
    uart_puts("{");uart_puts(" ");
    print_set(obj);uart_puts(" ");
    uart_puts("}");uart_puts(" ");
  }else if (!obj_cmp((word*)o->type, cell_type)){
    uart_puts("(");uart_puts(" ");
    rec_obj_print((word*)o->contents[0]);
    rec_obj_print((word*)o->contents[1]);
    uart_puts(")");uart_puts(" ");
  }else if (!obj_cmp((word*)o->type, function_type)){
    //print_set((word*)o->contents[0]);
    uart_puts("<(");uart_puts(" ");
    rec_obj_print((word*)o->contents[1]);
    rec_obj_print((word*)o->contents[2]);
    uart_puts(")>");uart_puts(" ");
  }else if (!obj_cmp((word*)o->type, subarray_type)){
    uart_puts("subarray [");uart_puts(" ");;
    for (word i = 0; i < o->size; ++i){
      rec_obj_print((word*)o->contents[i]);
    }
    uart_puts("]");uart_puts(" ");
  }else if (!obj_cmp((word*)o->type, bcode_type)){
    uart_puts("<(");
    for (word i = 0; i < o->size; ++i){
      uart_puts("0x");
      uart_padded_uint(o->contents[i], 16, sizeof(word)*2);
      uart_puts(", ");
    }
    uart_puts(")>");
  }else{
    uart_puts("???");uart_puts(" ");
  }
}


void _object_delete(word * heap, word * obj){
  //rec_obj_print(obj);nl(1);
  free(heap, obj + 1);
}


void object_delete(word * heap, word * obj){
  Object *o = (Object *)obj;
  if (!o){return;}

  word * type = (word*)o->type;
  if (--(o->refcount)){
    return;
  }
  if (!obj_cmp(type, num_type) || !obj_cmp(type, string_type) || !obj_cmp(type, bcode_type)){
    _object_delete(heap, obj);
  }else if(!obj_cmp(type, set_type)){
    set_delete(heap, obj);
  }else if(!obj_cmp(type, cell_type) || !obj_cmp(type, subarray_type)){
    for (word i = 0; i < o->size; ++i){
      object_delete(heap, (word*)(o->contents[i]));
    }
    _object_delete(heap, obj);
  }else if (!obj_cmp(type, array_type)){
    obj_array_delete(heap, obj);
  }else if (!obj_cmp(type, function_type)){
    return;
  }
  object_delete(heap, type);
}


void o_del(word * heap, word * obj){
  ++((Object*)obj)->refcount;
  object_delete(heap, obj);
}


word * obj_array(word * heap, word size){
  word * arr = object(heap, subarray_type, size, (word*)0, 0);
  if (!arr){return (word*)0;}
  ++((Object*)arr)->refcount;
  word * ret = object(heap, array_type, 1, 0, 0);
  if (!ret){
    object_delete(heap, arr);
    return (word*)0;
  }
  ((Object*)ret)->contents[0] = (word)arr;
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
  object_delete(heap, (word*)((Object*)arr)->type);
  _object_delete(heap, obj);
  if (--arr->refcount){
    return;
  }
  for (word i = 0; i < arr->size; ++i){
    object_delete(heap, (word*)(arr->contents[i]));
  }
  _object_delete(heap, (word*)arr);
}


void _obj_array_flatten(word * heap, word * ret, word * obj){
  word size = obj_array_size(obj);
  for (word i = 0; i < size; ++i){
    word * exp_i = obj_array_idx(obj, i);
    if (!obj_cmp((word*)((Object*)exp_i)->type, array_type)){
      _obj_array_flatten(heap, ret, exp_i);
    }else{
      obj_array_append(heap, ret, exp_i);
    }
  }
}


word * obj_array_flatten(word * heap, word * obj){
  word * ret = obj_array(heap, 16);
  _obj_array_flatten(heap, ret, obj);
  return ret;
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


word * array_resize(word * heap, word * arr, word new_size){
  word * mem_addr = arr - Array_bsz;
  Array * handle = (Array*)(mem_addr - 1);
  mem_addr = realloc(heap, mem_addr, Array_bsz + new_size * handle->item_sz);
  if (!mem_addr){return (word*)0;}
  return mem_addr + Array_bsz;
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
  return (array->mem_sz - Array_bsz - 1) / array->item_sz;
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
  if (bf == 1){
    return bf;
  }
  return (word)(bf - 3);
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
  if (!tree){return 0;}
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
  //AVL_Node ** tree = (AVL_Node**)tr;
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
  //word bf = 0;
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
    AVL_Node * l_child;
    AVL_Node * r_child;
    switch(bf){
    case -2:
      l_child = (AVL_Node*)parent->left;
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
      r_child = (AVL_Node*)parent->right;
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


void _print_avl(word * tree, word space){
  AVL_Node * node = (AVL_Node*)tree;
  if (!tree){return;}
  for (word i = 0; i < space; ++i){
    uart_puts(" ");
  }
  uart_print_uint(tree[0] & 3, 16);//nl(1);
  uart_puts(" ");uart_print_uint(tree[3], 16);uart_puts("\n");
  //spc(1);print_uint(tree, 16, 8);nl(1);
  word inc = 2;
  _print_avl((word*)(node->right), space + inc);
  _print_avl((word*)(node->left), space);
}


void print_avl(word * tree){
  if (!tree){uart_print_uint(0, 16);uart_puts("\n");}
  _print_avl(tree, 0);
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


void queue_print(word * queue){
  Queue * q = (Queue*)queue;
  Link * l = (Link*)q->first;
  while (l){
    uart_print_uint(l->next, 16);uart_puts(" ");
    uart_print_uint(l->prev, 16);uart_puts(" ");
    rec_obj_print((word*)l->data);uart_puts("\n");
    l = (Link*)l->next;
  };
}


word * ring_buf(word * heap, word size, word item_sz){
  if (size <= 1){return 0;}
  RingBuf * rb = (RingBuf*)alloc(heap, sizeof(RingBuf)/sizeof(word));
  if (!rb){return 0;}
  word * arr = array(heap, size, item_sz);
  if (!arr){
    free(heap, (word*)rb);
    return 0;
  }
  rb->buf = arr;
  rb->head = 0;
  rb->tail = 0;
  return (word*)rb;
}


word ring_buf_isempty(word * ringbuf){
  RingBuf * rb = (RingBuf*)ringbuf;
  return (word)(rb->head == rb->tail);
}


word ring_buf_isfull(word * ringbuf){
  RingBuf * rb = (RingBuf*)ringbuf;
  word sz = array_capacity((word*)rb->buf);
  return (rb->tail + (sz - 1)) % sz == rb->head;
}


word ring_buf_item_sz(word * ringbuf){
  RingBuf * rb = (RingBuf*)ringbuf;
  return ((Array*)(rb->buf - Array_bsz - 1))->item_sz;
}


void ring_buf_add(word * ringbuf, word * item){
  RingBuf * rb = (RingBuf*)ringbuf;
  word sz = array_capacity((word*)rb->buf);
  word item_sz = ring_buf_item_sz(ringbuf);
  word * ptr = (word*)rb->buf + item_sz * rb->head;
  for (word i = 0; i < item_sz; ++i){
    ptr[i] = item[i];
  }
  ++rb->head;
  if (rb->head >= sz){rb->head = 0;}
}


void ring_buf_pop(word * ringbuf, word * item){
  RingBuf * rb = (RingBuf*)ringbuf;
  word sz = array_capacity((word*)rb->buf);
  word item_sz = ring_buf_item_sz(ringbuf);
  word * ptr = (word*)rb->buf + item_sz * rb->tail;
  if (item){
    for (word i = 0; i < item_sz; ++i){
      item[i] = ptr[i];
    }
  }
  ++rb->tail;
  if (rb->tail >= sz){rb->tail = 0;}
}


void ring_buf_delete(word * heap, word * ringbuf){
  RingBuf * rb = (RingBuf*)ringbuf;
  array_delete(heap, rb->buf);
  free(heap, ringbuf);
}


void ring_buf_print(word * ring_buf){
  RingBuf * rb = (RingBuf*)ring_buf;
  uart_print_uint(ring_buf_isfull((word*)rb), 16);uart_puts(" ");
  uart_print_uint(rb->head, 16);uart_puts(" ");
  uart_print_uint(rb->tail, 16);uart_puts("\n");
  word sz = array_capacity(rb->buf);
  word item_sz = ring_buf_item_sz(ring_buf);
  for (word i = rb->tail; i != rb->head; ++i, i %= sz){
    for (word j = 0; j < item_sz; ++j){
      uart_print_uint(rb->buf[i*item_sz+j], 16);uart_puts(" ");
    }
  }uart_puts("\n");
}


word * concurrent_fifo(word * heap, word primary_capacity, word secondary_capacity, word item_sz){
  ConcurrentFIFO * cf = (ConcurrentFIFO*)alloc(heap, sizeof(ConcurrentFIFO)/sizeof(word));
  if (!cf){goto error_0;}
  cf->A = (RingBuf*)ring_buf(heap, primary_capacity, item_sz + 1);
  if (!cf->A){goto error_1;}
  cf->B = (RingBuf*)ring_buf(heap, secondary_capacity, item_sz + 1);
  if (!cf->B){goto error_2;}
  cf->a_lck = 0;
  cf->b_lck = 0;
  cf->tag = 0;
  cf->curr_tag = 0;
  return (word*)cf;
 error_2: free(heap, (word*)(cf->A));
 error_1: free(heap, (word*)cf);
 error_0: return 0;
}


word concurrent_fifo_isempty(word * c_fifo){
  ConcurrentFIFO * cf = (ConcurrentFIFO*)c_fifo;
  return ring_buf_isempty((word*)cf->A) && ring_buf_isempty((word*)cf->B);
}


word concurrent_fifo_isfull(word * c_fifo){
  ConcurrentFIFO * cf = (ConcurrentFIFO*)c_fifo;
  return ring_buf_isfull((word*)cf->A) && ring_buf_isfull((word*)cf->B);
}


word concurrent_fifo_item_sz(word * c_fifo){
  ConcurrentFIFO * cf = (ConcurrentFIFO*)c_fifo;
  return ring_buf_item_sz((word*)cf->A);
}


word concurrent_fifo_add(word * c_fifo, word * item){
  ConcurrentFIFO * cf = (ConcurrentFIFO*)c_fifo;
  word item_sz = ring_buf_item_sz((word*)cf->A);
  word tagged_item[item_sz];
  tagged_item[0] = (cf->tag)++;
  --item;
  for (word i = 1; i < item_sz; ++i){
    tagged_item[i] = item[i];
  }

  if (!cf->a_lck && !ring_buf_isfull((word*)cf->A)){
      ring_buf_add((word*)cf->A, tagged_item);
      return 0;
  }else if (!cf->b_lck && !ring_buf_isfull((word*)cf->B)){
      ring_buf_add((word*)cf->B, tagged_item);
      return 0;
  }
  // Only fail if all the buffers are full/inaccessible at the same time.
  --(cf->tag);
  return 1;
}


word concurrent_fifo_pop(word * c_fifo, word * item){
  ConcurrentFIFO * cf = (ConcurrentFIFO*)c_fifo;
  word item_sz = ring_buf_item_sz((word*)cf->A);
  word tagged_item[item_sz];
  RingBuf * A = (RingBuf*)cf->A;
  RingBuf * B = (RingBuf*)cf->B;
  word * A_ptr = (word*)A->buf + item_sz * A->tail;
  word * B_ptr = (word*)B->buf + item_sz * B->tail;
  word c_tag = cf->curr_tag;

  if (!ring_buf_isempty((word*)A) && c_tag == A_ptr[0]){
    cf->a_lck = 1;
    ring_buf_pop((word*)A, tagged_item);
    cf->a_lck = 0;
  }else if (!ring_buf_isempty((word*)B) && c_tag == B_ptr[0]){
    cf->b_lck = 1;
    ring_buf_pop((word*)B, tagged_item);
    cf->b_lck = 0;
  }else{
    return 1;
  }

  --item;
  for (word i = 1; i < item_sz; ++i){
    item[i] = tagged_item[i];
  }
  ++(cf->curr_tag);
  return 0;
}


void concurrent_fifo_delete(word * heap, word * c_fifo){
  ConcurrentFIFO * cf = (ConcurrentFIFO*)c_fifo;
  ring_buf_delete(heap, (word*)cf->A);
  ring_buf_delete(heap, (word*)cf->B);
  free(heap, c_fifo);
}


void concurrent_fifo_print(word * c_fifo){
  ConcurrentFIFO * cf = (ConcurrentFIFO*)c_fifo;
  uart_puts("\n");
  uart_print_uint(cf->a_lck, 16);uart_puts("  ");
  uart_print_uint(cf->b_lck, 16);uart_puts("  ");
  uart_print_uint(cf->curr_tag, 16);uart_puts("  ");
  uart_print_uint(cf->tag, 16);uart_puts("  \n");
  ring_buf_print((word*)cf->A);
  ring_buf_print((word*)cf->B);
}


void init_types(word * heap){
  string_type = cstr_to_string(heap, "str");
  ((Object*)string_type)->type = (word)string_type;
  ((Object*)string_type)->refcount += 2;
  num_type = cstr_to_string(heap, "num");
  ++((Object*)num_type)->refcount;
  array_type = cstr_to_string(heap, "arr");
  ++((Object*)array_type)->refcount;
  subarray_type = cstr_to_string(heap, "sar");
  ++((Object*)subarray_type)->refcount;
  set_type = cstr_to_string(heap, "set");
  ++((Object*)set_type)->refcount;
  function_type = cstr_to_string(heap, "fun");
  ++((Object*)function_type)->refcount;
  cell_type = cstr_to_string(heap, "cel");
  ++((Object*)cell_type)->refcount;
  bcode_type = cstr_to_string(heap, "bco");
  ++((Object*)bcode_type)->refcount;
  /* word native_str[3] = {'n', 't', 'v'}; */
  /* native_type = object(heap, string_type, 3, native_str, 3); */
  /* word asm_str[3] = {'a', 's', 'm'}; */
  /* asm_type = object(heap, string_type, 3, asm_str, 3); */
}


//NOTE: From this point forward only use if types are initialized.

word * pair(word * heap, word * obj1, word * obj2){
  Object * o1 = (Object *)obj1;
  Object * o2 = (Object *)obj2;
  Object * obj = (Object*)object(heap, cell_type, 2, 0, 0);
  if (!obj){return 0;}
  obj->size = 2;
  obj->contents[0] = (word)obj1;
  obj->contents[1] = (word)obj2;
  o1->refcount += 1;
  o2->refcount += 1;
  return (word*)obj;
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


word * cstr_to_string(word * heap, char * str){
  Object * obj = (Object*)object(heap, string_type, 8, 0, 0);
  for (word i = 0; str[i]; ++i){
    obj = (Object*)object_append_word(heap, (word*)obj, (word)str[i]);
  }
  return (word*)obj;
}


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

  // Create an obj of size n1_sz + 1 because n1 is now the "longest" number.
  Object * result = (Object*)object(heap, num_type, n1_sz + 1, 0, 0);
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
  //TODO: if shorter number is positive can exit earlier
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
    for (word i = n1_sz-1; i > 1 && (result->contents[i-1] == last); --i){
      --result->size;
    }
  }

  return (word*)result;
}


word * num_shift_left(word * heap, word * num, word shf){
  Object * n = ((Object*)num);
  word n_sz = n->size;
  word n_sgn = n->contents[n_sz-1];
  word div = shf / (sizeof(word)*8);
  word mod = shf % (sizeof(word)*8);
  // If mod need an extra word + another for possibly adding a sign word
  word min_sz = n_sz + div + 2*!!mod;
  if (n->max_sz < min_sz && !(n = (Object*)object_expand(heap, num, min_sz))){
    return 0;
  }else{
    // Subtract !!mod so can append a sign word if mod
    n->size = min_sz - !!mod;
  }

  if (mod){
    word op_mod = ((sizeof(word)*8)-mod);
    for (word i = n_sz-1; i > 0; --i){
      n->contents[i+div] = (n->contents[i-1] >> op_mod) | (n->contents[i] << mod);
    }
    n->contents[div] = n->contents[0] << mod;

    word last = n_sz + div;
    n->contents[last] = n_sgn >> op_mod;
    if (n_sgn){
      n->contents[last+1] = 0;
      ++n->size;
    }
    for (word i = n->size-1; i > 1 && !(n->contents[i-1]); --i){--n->size;}
    if (n->size > 2 && !n->contents[n->size-1] && n->contents[n->size-2] == (word)-1){--n->size;}
  }else if (div){
    for (word i = n_sz; i; --i){
      word i1 = i-1;
      n->contents[i1+div] = n->contents[i1];
    }
  }
  for (word i = 0; i < div; ++i){n->contents[i] = 0;}
  return (word*)n;
}


word * num_shift_right(word * heap, word * num, word shf){
  Object * n = ((Object*)num);
  word n_sz = n->size;
  word n_sgn = n->contents[n_sz-1];
  word div = shf / (sizeof(word)*8);
  word mod = shf % (sizeof(word)*8);
  if (shf >= n_sz*sizeof(word)*8){
    n->contents[0] = 0;
    n->contents[1] = 0;
    n->size = 2;
    return (word*)n;
  }
  if (mod){
    word op_mod = ((sizeof(word)*8)-mod);
    for (word i = 0; i < n_sz - div - 1; ++i){
      n->contents[i] = (n->contents[i+div] >> mod) | (n->contents[i+div+1] << op_mod);
    }

    n->size -= div;
    n->contents[n->size-1] = n_sgn >> mod;
    // num gets deformed if sgn or div == n_sz - 1
    if (n_sgn || div >= n_sz - 1){
      n = (Object*)object_append_word(heap, (word*)n, 0);
    }

    // Get rid of unnecessary zeroes
    for (word i = n->size-1; i > 1 && !(n->contents[i-1]); --i){--n->size;}
    // Check if number should be negative
    if (n->size > 2 && !n->contents[n->size-1] && n->contents[n->size-2] == (word)-1){--n->size;}
  }else if (div){
    for (word i = 0; i < n_sz-div; ++i){
      n->contents[i] = n->contents[i+div];
    }
    n->size -= div;
  }
  return (word*)n;
}


word num_le(word * heap, word * num1, word * num2){
  num_negate(num2);
  word * num3 = num_add(heap, num1, num2);
  num_negate(num2);

  Object * n3 = ((Object*)num3);
  word n_sgn = n3->contents[n3->size-1];
  ++n3->refcount;
  object_delete(heap, num3);
  return n_sgn;
}


word * num_shift(word * heap, word * num1, word * num2){
  Object * n1 = (Object*)num1;
  Object * n2 = (Object*)num2;
  word n2_sz = n2->size;
  word n2_sgn = n2->contents[n2_sz-1];

  // If shift is zero return
  if (n2_sz <= 2 && !n2->contents[0]){
    return num1;
  }else if (n2_sz > 2){
    // Return if num2 is too large
    return num1;
  }

  word * result = 0;
  if (n2_sgn){
    num_negate(num2);
    word sz = n1->size + n2->contents[0]/(sizeof(word)*8) + 1 + 1;
    result = object(heap, num_type, sz, n1->contents, n1->size);
    num_shift_left(heap, result, n2->contents[0]);
    num_negate(num2);
  }else{
    word sz = n1->size + 1;
    result = object(heap, num_type, sz, n1->contents, n1->size);
    num_shift_right(heap, result, n2->contents[0]);
  }
  
  return result;
}


word * num_and(word * heap, word * num1, word * num2){
  Object * n1 = (Object*)num1;
  Object * n2 = (Object*)num2;
  word sz = umin(n1->size, n2->size);
  Object * result = (Object*)object(heap, num_type, sz + 1, n1->contents, sz);
  for (word i = 0; i < sz; ++i){
    result->contents[i] &= n2->contents[i];
  }
  result = (Object*)object_append_word(heap, (word*)result, 0);
  for (word i = result->size-1; i > 1 && !(result->contents[i-1]); --i){--result->size;}
  if (result->size > 2 && !result->contents[result->size-1] && result->contents[result->size-2] == (word)-1){--result->size;}
  return (word*)result;
}


word * num_or(word * heap, word * num1, word * num2){
  Object * n1 = (Object*)num1;
  Object * n2 = (Object*)num2;
  if (n1->size < n2->size){
    Object * tmp = n1;
    n2 = n1;
    n1 = tmp;
  }
  Object * result = (Object*)object(heap, num_type, n1->size, n1->contents, n1->size);
  for (word i = 0; i < n2->size; ++i){
    result->contents[i] |= n2->contents[i];
  }
  return (word*)result;
}


word * num_xor(word * heap, word * num1, word * num2){
  Object * n1 = (Object*)num1;
  Object * n2 = (Object*)num2;
  if (n1->size < n2->size){
    Object * tmp = n1;
    n2 = n1;
    n1 = tmp;
  }
  Object * result = (Object*)object(heap, num_type, n1->size, n1->contents, n1->size);
  for (word i = 0; i < n2->size; ++i){
    result->contents[i] ^= n2->contents[i];
  }
  return (word*)result;
}


word * num_not(word * heap, word * num){
  Object * n = (Object*)num;
  Object * result = (Object*)object(heap, num_type, n->size, n->contents, n->size);
  for (word i = 0; i < n->size; ++i){
    result->contents[i] = ~result->contents[i];
  }
  return (word*)result;
}


word * num_ldr(word * heap, word * num){
  Object * n = (Object*)num;
  return word_to_num(heap, *((word*)n->contents[0]));
}


void num_store(word * num1, word * num2){
  Object * n1 = (Object*)num1;
  Object * n2 = (Object*)num2;
  for (word i = 0; i < n2->size - 1; ++i){
    *((word*)n1->contents[i]) = n2->contents[i];
  }
}


word * num_mul(word * heap, word * num1, word * num2){
  Object * n1 = (Object*)num1;
  Object * n2 = (Object*)num2;

  // Put the "longer" number last. 
  if (n1->size > n2->size){
    Object * tmp = n1;
    n1 = n2;
    n2 = tmp;
  }

  word n1_sz = n1->size;
  word n2_sz = n2->size;
  word res_sz = n1_sz + n2_sz - 1; // Only need one sign word, so subtract 1
  word n1_sgn = n1->contents[n1_sz-1];
  word n2_sgn = n2->contents[n2_sz-1];
  word res_sgn = n1_sgn ^ n2_sgn;

  if ((n1_sz == 2 && !n1->contents[0]) || (n1_sz == 2 && !n1->contents[0])){
    word buf[2];
    buf[0] = buf[1] = 0;
    return object(heap, num_type, 2, buf, 2);
  }

  if (n1_sgn){num_negate((word*)n1);}
  if (n2_sgn){num_negate((word*)n2);}

  Object * result = (Object*)object(heap, num_type, res_sz, 0, 0);
  result->size = res_sz;
  for (word i = 0; i < res_sz; ++i){result->contents[i] = 0;}

  for (word i = 0; i < n2_sz-1; ++i){
    word n2_i = n2->contents[i];
    word nsh = 0;
    while(n2_i){
      if (n2_i & 1){
	word * ptr = result->contents + i;
	word carry = 0;
	word n1_prev = 0;
	// Always adding two positive integers, where the accumulator is the strictly smaller number.
	for (word j = 0; j < n1_sz; ++j, ++ptr){
	  word n1_curr = n1->contents[j];
	  carry = adc(*ptr, carry, ptr);
	  carry += adc(*ptr, (nsh ? (n1_prev >> (sizeof(word)*8 - nsh)) : 0) | (n1_curr << nsh), ptr);
	  n1_prev = n1_curr;
	}
      }
      ++nsh;
      n2_i >>= 1;
    }
  }

  if (n1_sgn){num_negate((word*)n1);}
  if (n2_sgn){num_negate((word*)n2);}
  if (res_sgn){num_negate((word*)result);}
  for (word i = res_sz - 1; i > 1 && (result->contents[i-1] == res_sgn); --i){--result->size;}
  return (word *)result;
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
    else {
      --((Object*)num_type)->refcount;
      _object_delete(heap, (word*)final_num); return 0;
    }
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
    uart_print_uint(n->contents[0], 16);
  }else if (n->size > 1){
    word last = n->contents[n->size - 1];
    if (last){
      uart_puts("-");
      num_negate(num);
    }
    uart_print_uint(n->contents[n->size - 2], 16);
    for (word i = n->size - 2; i > 0; --i){
      uart_padded_uint(n->contents[i-1], 16, sizeof(word)*2);
    }
    if (last){
      num_negate(num);
    }
  }
}


word * word_to_num(word * heap, word w){
  Object * obj = (Object*)object(heap, num_type, 2, 0, 0);
  obj->contents[0] = w;
  obj->contents[1] = 0;
  obj->size = 2;
  return (word*)obj;
}


word * num_to_str(word * heap, word * num){
  Object * n = (Object*)num;
  Object * string = (Object*)object(heap, string_type, 2*sizeof(word) * n->size + 1, 0, 0);
  if (!string){return 0;}
  word buf[2*sizeof(word)+1];
  word last = n->contents[n->size - 1];

  if (last){
    string = (Object*)object_append_word(heap, (word*)string, '-');
    num_negate(num);
  }

  word l = uintn_to_str(buf, sizeof(buf)/sizeof(word), n->contents[n->size - 2], 2*sizeof(word));
  for (word j = 0; j < l; ++j){
    string = (Object*)object_append_word(heap, (word*)string, buf[j]);
  }
  for (word i = n->size - 2; i > 0; --i){
    l = uintn_to_str(buf, sizeof(buf)/sizeof(word), n->contents[i-1], 2*sizeof(word));
    for (word j = l; j < 2*sizeof(word); ++j){
      string = (Object*)object_append_word(heap, (word*)string, '0');
    }
    for (word j = 0; j < l; ++j){
      string = (Object*)object_append_word(heap, (word*)string, buf[j]);
    }
  }

  if (last){
    num_negate(num);
  }

  return (word*)string;
}
