#include "config.h"

//NOTE: Some of the datastructures here depend on the memory allocator leaving the size of the allocated memory just before the mem.

// Note: Use black magic to get maximum size from the umds associated with the object when expanding; No sense consuming another word.
// TODO: Refcount attributes, e.g. cyclic (also add in manual ability to deallocate or cycle check)


word * num_type;
word * string_type;
word * array_type;
word * set_type;
word * function_type;
word * cell_type;


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


typedef struct avl_node{
  word prev;
  word left;
  word right;
  word data;
} AVL_Node;

const word avl_node_sz = sizeof(AVL_Node)/sizeof(word);


word get_avl_node_sz(void){
  return avl_node_sz;
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


void avl_merge(word ** tr, word * addr, word size){
  word * amax = addr + size;
  AVL_Node * left = (AVL_Node*)0;
  AVL_Node * right = (AVL_Node*)0;
  AVL_Node * tree = (AVL_Node*)*tr;
  while (tree && (!left || !right)){
    if ((word*)tree == amax){
      right = (AVL_Node*)tree;
      tree = (AVL_Node*)(tree->left);
    }
    if ((word*)tree + tree->data == addr){
      left = (AVL_Node*)tree;
      tree = (AVL_Node*)(tree->right);
    }
    
    if ((word*)tree > amax){
      tree = (AVL_Node*)(tree->left);
    }else if ((word*)tree + tree->data < addr){
      tree = (AVL_Node*)(tree->right);
    }
  }

  if (right){
    size += right->data;
    _avl_delete(tr, right->data, &avl_basic_cmp);
  }
  if (left){
    addr -= left->data;
    size += left->data;
    _avl_delete(tr, left->data, &avl_basic_cmp);
  }

  _avl_insert(tr, addr, size, &avl_basic_cmp);
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


word avl_min_ge(word * tree, word data){
  if (!tree){return 0;}
  AVL_Node * prev = (AVL_Node*)0;
  for (; ((AVL_Node*)tree)->data >= data; tree = (word*)(((AVL_Node*)tree)->left)){
    prev = (AVL_Node*)tree;
  }
  return prev->data;
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

  // Insert node.
  // If b is zero, node is already in tree, so it can be used to exit the while loop.
  for (word b = cmp((word*)tree, (word*)node); b; b = cmp((word*)tree, (word*)node)){
    node->prev = (word)tree | 3;
    switch (b){
    case -1:
      if (!tree->left){
	tree->left = (word)node;
	b = 0;
	break;
      }
      tree = (AVL_Node*)(tree->left);
      break;
    case 0:
      return 1;
    case 1:
      if (!tree->right){
	tree->right = (word)node;
	b = 0;
	break;
      }
      tree = (AVL_Node*)(tree->right);
      break;
    }
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
    }else{
      set_balance_factor((word*)node, bf);
    }
    child = node;
  }

  return 0;
}


word avl_insert(word * heap, word ** tr, word data, word (*cmp)(word*, word*)){
  AVL_Node ** tree = (AVL_Node**)tr;
  word * node = alloc(heap, avl_node_sz);
  if (!node){
    return 1;
  }

  word ret = _avl_insert(tr, node, data, cmp);
  if (ret){
    free(heap, (word*)node);
  }
  return ret;
}


word * _avl_delete(word ** tr, word data, word (*cmp)(word*, word*)){
  AVL_Node * tree = (AVL_Node*)*tr;
  AVL_Node node;
  node.data = data;

  word * ret = 0;
  if (!tree){return ret;}

  // Find node
  for (word b = cmp((word*)tree, (word*)&node); b; b = cmp((word*)tree, (word*)&node)){
    switch (b){
    case -1:
      tree = (AVL_Node*)(tree->left);
    case 0:
      break;
    case 1:
      tree = (AVL_Node*)(tree->right);
    }
    if (!tree){return ret;}
  }

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
  AVL_Node * parent = get_parent(successor);
  if (successor){
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
  word * x = _avl_delete(tr, data, cmp);
  if (x){
    free(heap, x);
    return 0;
  }
  return 1;
}


word avl_basic_cmp(word * n1, word * n2){
  AVL_Node * node1 = (AVL_Node*)n1;
  AVL_Node * node2 = (AVL_Node*)n2;

  if (node1->data <= node2->data){
    if (node1->data == node2->data){
      return (word)0;
    }
    return (word)1;
  }
  return (word)-1;
}


void print_avl(word * tree, word space, word inc){
  AVL_Node * node = (AVL_Node*)tree;
  if (!tree){return;}
  //spc(space);print_uint(tree, 16, 8);nl(1);
  spc(space);print_uint(tree[0] & 3, 16, 2);//nl(1);
  spc(1);print_uint(tree[3], 16, 2);nl(1);
  print_avl((word*)(node->right), space + inc, inc);
  print_avl((word*)(node->left), space, inc);
}


word * check_balance_factors(word * tr){
  AVL_Node * tree = (AVL_Node*)tr;
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


//NOTE: From this point forward only use if types are initialized.

word * pair(word * heap, word * obj1, word * obj2){
  word contents[2] = {(word)obj1, (word)obj2};
  Object * o1 = (Object *)obj1;
  Object * o2 = (Object *)obj2;
  o1->refcount += 1;
  o2->refcount += 1;
  return object(heap, cell_type, 2, contents, 2);
}


word pair_strcmp(word * pair1, word * pair2){
  Object * p1 = (Object*)pair1;
  Object * p2 = (Object*)pair2;
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
    }else{
      return 0;
    }  
  }
  return 0; // To silence warning.
}


word * set(word * heap){
  word contents = 0;
  return object(heap, set_type, 1, &contents, 1);
}


word set_add(word * heap, word * s, word * data, word (*cmp)(word*, word*)){
  Object * obj = (Object*)s;
  Object * d_obj = (Object*)data;
  d_obj->refcount += 1;
  return avl_insert(heap, (word**)&(obj->contents), (word)data, cmp);
}


word set_add_str_key(word * heap, word * s, word * key, word * val){
  word * data = pair(heap, key, val);
  return set_add(heap, s, data, &pair_strcmp);
}

