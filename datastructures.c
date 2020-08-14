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


typedef struct avl_node{
  word prev;
  word left;
  word right;
  word data;
} AVL_Node;

const word avl_node_sz = sizeof(AVL_Node)/sizeof(word);


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


word avl_rotate_left(word ** tr, word * nd){
  AVL_Node * node = (AVL_Node*)nd; 
  if (!node){return 1;} 
  AVL_Node * parent = get_parent(node);
  AVL_Node * root = (AVL_Node*)(node->right);
  word old_rootbf = balance_factor((word*)root);
  if (!root){return 1;}
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

  return 0;
}


word avl_rotate_right(word ** tr, word * nd){
  AVL_Node * node = (AVL_Node*)nd; 
  if (!node){return 1;} 
  AVL_Node * parent = get_parent(node);
  AVL_Node * root = (AVL_Node*)(node->left);
  word old_rootbf = balance_factor((word*)root);
  if (!root){return 1;}
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

  return 0;
}


word avl_rotate_lr(word ** tr, word * node){
  AVL_Node * child = (AVL_Node*)(((AVL_Node*)node)->left);
  AVL_Node * lr_child = (AVL_Node*)(child->right);
  word bf = balance_factor((word*)lr_child);
  avl_rotate_left(tr, (word*)child);
  avl_rotate_right(tr, (word*)node);
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
  return 0;
}


word avl_rotate_rl(word ** tr, word * node){
  AVL_Node * child = (AVL_Node*)(((AVL_Node*)node)->right);
  AVL_Node * rl_child = (AVL_Node*)(child->left);
  word bf = balance_factor((word*)rl_child);
  avl_rotate_right(tr, (word*)child);
  avl_rotate_left(tr, (word*)node);
  if (bf == 1) {
    set_balance_factor((word*)node, (word)-1);
    set_balance_factor((word*)child, 0);
  }else if (bf == (word)-1){
    set_balance_factor((word*)child, (word)1);
    set_balance_factor((word*)node, 0);
  }else{
    set_balance_factor((word*)child, 0);
    set_balance_factor((word*)node, 0);
  }
  word * pp = (word*)get_parent((AVL_Node*)node);
  set_balance_factor(pp, 0);
  return 0;
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

  
  AVL_Node * successor = (AVL_Node*)(tree->left);
  if (tree->right){
    successor = (AVL_Node*)(tree->right);
    while (successor->left){
      successor = (AVL_Node*)successor->left;
    }
  }

  AVL_Node * child = 0;
  word bf = 0;
  // Will start rebalancing tree from parent later.
  AVL_Node * parent = get_parent(successor);
  word pbf = balance_factor((word*)parent);
  if (successor){
    // successor must have a parent if it exists.
    tree->data = successor->data;
    child = (AVL_Node*)(successor->right);
    word cbf = get_balance_factor((word*)child);
    if (child){((AVL_Node*)child)->prev = (word)parent | cbf;}
    if (parent->left == (word)successor){
      parent->left = successor->right;
    }else{
      parent->right = successor->right;
    }
    ret = (word*)successor;
  }else{
    parent = get_parent(tree);
    pbf = balance_factor((word*)parent);
    if (!parent){
      *tr = (word*)0;
      ret = (word*)tree;
      return ret;
    }
    // tree must have a parent at this point, and tree must have no children. (tree is a leaf node)

    if (parent->left == (word)tree){
      parent->left = 0;
    }else if (parent->right == (word)tree){
      parent->right = 0;
    }
    ret = (word*)tree;
  }

  AVL_Node * pp = 0;
  for (; parent; parent = pp){
    pp = get_parent(parent);
    word bf = balance_factor((word*)parent);
    word _bf = 0;
    // Check right first in case child is null.
    if ((word)child == parent->right){
      if (bf == (word)-1){
    	AVL_Node * l_child = (AVL_Node*)parent->left;
    	_bf = balance_factor((word*)l_child);
    	if (_bf == 1){
    	  avl_rotate_lr(tr, (word*)parent);
    	}else{
    	  avl_rotate_right(tr, (word*)parent);
    	}
      }else{
    	if (bf == 0){
    	  set_balance_factor((word*)parent, -1);
    	  break;
    	}
    	set_balance_factor((word*)parent, 0);
        child = parent;
    	continue;
      }
    }else{
      AVL_Node * r_child = (AVL_Node*)parent->right;
      _bf = balance_factor((word*)r_child);
      if (bf == 1){
    	if (_bf == (word)-1){
    	  avl_rotate_rl(tr, (word*)parent);
    	}else{
    	  avl_rotate_left(tr, (word*)parent);
    	}
      }else{
    	if (bf == 0){
    	  set_balance_factor((word*)parent, 1);
    	  break;
    	}
    	set_balance_factor((word*)parent, 0);
    	child = parent;
    	continue;
      }
    }
    if (!_bf){break;}
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
