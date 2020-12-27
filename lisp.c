#include "config.h"

void _object_delete(word * heap, word * obj);

word * def = (word*)0;
word * call = (word*)0;
word * reduce = (word*)0;
word * name = (word*)0;
word * blk_l_delim = (word*)0;
word * blk_r_delim = (word*)0;
word * str_delim = (word*)0;

word * space = (word*)0;
word * newline = (word*)0;
word * tab = (word *)0;
word * null = (word*)0;


word * init_machine(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE * SystemTable){
  word * machine = set(global_heap_start);
  ((Object*)machine)->refcount += 1;

  word st_key[7] = {'s', 'y', 's', '_', 't', 'a', 'b'};
  word * st_str = object(global_heap_start, string_type, 7, st_key, 7);
  word * st_val = object(global_heap_start, num_type, 1, (word*)SystemTable, 1);
  set_add_str_key(global_heap_start, machine, st_str, st_val);

  word img_key[3] = {'i', 'm', 'g'};
  word * img_str = object(global_heap_start, string_type, 3, img_key, 3);
  word * img_val = object(global_heap_start, num_type, 1, (word*)ImageHandle, 1);
  set_add_str_key(global_heap_start, machine, img_str, img_val);

  word types_key[5] = {'t', 'y', 'p', 'e', 's'};
  word * types_str = object(global_heap_start, string_type, 5, types_key, 5);
  word * types_set = set(global_heap_start);
  set_add_str_key(global_heap_start, types_set, string_type, string_type);
  set_add_str_key(global_heap_start, types_set, num_type, num_type);
  set_add_str_key(global_heap_start, types_set, array_type, array_type);
  set_add_str_key(global_heap_start, types_set, set_type, set_type);
  set_add_str_key(global_heap_start, types_set, function_type, function_type);
  set_add_str_key(global_heap_start, types_set, cell_type, cell_type);
  /* set_add_str_key(global_heap_start, types_set, native_type, native_type); */
  set_add_str_key(global_heap_start, machine, types_str, types_set);

  word pc_key[2] = {'p', 'c'};
  word * pc_str = object(global_heap_start, string_type, 2, pc_key, 2);
  word * pc_set = set(global_heap_start);
  set_add_str_key(global_heap_start, machine, pc_str, pc_set);

  word fb_set_key[2] = {'f', 'b'};
  word * fb_set_str = object(global_heap_start, string_type, 2, fb_set_key, 2);
  word * fb_set = set(global_heap_start);
  set_add_str_key(global_heap_start, pc_set, fb_set_str, fb_set);
  word fb_base_key[4] = {'b', 'a', 's', 'e'};
  word * fb_base_str = object(global_heap_start, string_type, 4, fb_base_key, 4);
  word * fb_base_val = object(global_heap_start, num_type, 1, (word*)&fb_start, 1);
  word fb_hres_key[4] = {'h', 'r', 'e', 's'};
  word * fb_hres_str = object(global_heap_start, string_type, 4, fb_hres_key, 4);
  word * fb_hres_val = object(global_heap_start, num_type, 1, (word*)&b_hres, 1);
  word fb_vres_key[4] = {'v', 'r', 'e', 's'};
  word * fb_vres_str = object(global_heap_start, string_type, 4, fb_vres_key, 4);
  word * fb_vres_val = object(global_heap_start, num_type, 1, (word*)&b_vres, 1);
  set_add_str_key(global_heap_start, fb_set, fb_base_str, fb_base_val);
  set_add_str_key(global_heap_start, fb_set, fb_hres_str, fb_hres_val);
  set_add_str_key(global_heap_start, fb_set, fb_vres_str, fb_vres_val);

  word mem_key[3] = {'m', 'e', 'm'};
  word * mem_set_str = object(global_heap_start, string_type, 3, mem_key, 3);
  word * mem_set = set(global_heap_start);
  set_add_str_key(global_heap_start, pc_set, mem_set_str, mem_set);
  word * mem_base = object(global_heap_start, num_type, 1, (word*)&global_heap_start, 1);
  set_add_str_key(global_heap_start, mem_set, fb_base_str, mem_base);
  word mem_sz_key[3] = {'s', 'z'};
  word * mem_sz_str = object(global_heap_start, string_type, 2, mem_sz_key, 2);
  word * mem_sz = object(global_heap_start, num_type, 1, (word*)&global_heap_size, 1);
  set_add_str_key(global_heap_start, mem_set, mem_sz_str, mem_sz);

  return machine;
}


word * _tokenize(word * heap, word * code, word code_len, word * i, word * ws, word * ops){
  word * ret = obj_array(heap, 8);
  word _tmp[sizeof(Object)+1];
  Object * tmp = (Object*)&_tmp;
  *tmp = (Object){5, 1, (word)string_type, 1, {}};
  
  while (*i < code_len){
    word * tok = object(heap, string_type, 1, code + *i, 1);
    if (in_set(ws, tok)){
      _object_delete(heap, tok);
    }else if (in_set(ops, tok)){
      if (!obj_cmp((word*)tok, str_delim)){
	--((Object*)tok)->size;
	for(++*i; *i < code_len; ++*i){
	  tmp->contents[0] = code[*i];
	  if (!obj_cmp((word*)tmp, str_delim)){
	    break;
	  }
	  tok = object_append_word(heap, tok, code[*i]);
	}
      }else if (!obj_cmp((word*)tok, blk_l_delim)){
	_object_delete(heap, tok);
	++*i;
	obj_array_append(heap, ret, _tokenize(heap, code, code_len, i, ws, ops));
	continue;
      }else if (!obj_cmp((word*)tok, blk_r_delim)){
	_object_delete(heap, tok);
	++*i;
	return ret;
      }

      obj_array_append(heap, ret, tok);
    }else{
      for (++*i; *i < code_len; ++*i){
	tmp->contents[0] = code[*i];
	if (in_set(ws, (word*)tmp) || in_set(ops, (word*)tmp)){
	  --*i;
	  break;
	}
	tok = object_append_word(heap, tok, code[*i]);
      }

      word * b = str_to_num(heap, tok);
      if (b){
	_object_delete(heap, tok);
	tok = b;
      }
      obj_array_append(heap, ret, tok);
    }
    ++*i;
  }

  return ret;
}

word * tokenize(word * heap, word * code, word code_len){
  word set_cmp(word * node1, word * node2);
  word _def[1] = {'~'};
  def = object(heap, string_type, 1, _def, 1);
  word _call[1] = {':'};
  call = object(heap, string_type, 1, _call, 1);
  word _reduce[1] = {'>'};
  reduce = object(heap, string_type, 1, _reduce, 1);
  word _name[1] = {'|'};
  name = object(heap, string_type, 1, _name, 1);
  word _blk_l_delim[1] = {'['};
  blk_l_delim = object(heap, string_type, 1, _blk_l_delim, 1);
  word _blk_r_delim[1] = {']'};
  blk_r_delim = object(heap, string_type, 1, _blk_r_delim, 1);
  word _str_delim[1] = {'\''};
  str_delim = object(heap, string_type, 1, _str_delim, 1);
  
  word _space[1] = {' '};
  space = object(heap, string_type, 1, _space, 1);
  word _newline[1] = {'\n'};
  newline = object(heap, string_type, 1, _newline, 1);
  word _tab[1] = {'\t'};
  tab = object(heap, string_type, 1, _tab, 1);
  word _null[1] = {'\0'};
  null = object(heap, string_type, 1, _null, 1);

  word * ops = set(heap);
  Object * o = (Object*)ops;
  ++(o->refcount);
  set_add(heap, ops, def, set_cmp);
  set_add(heap, ops, call, set_cmp);
  set_add(heap, ops, reduce, set_cmp);
  set_add(heap, ops, name, set_cmp);
  set_add(heap, ops, blk_l_delim, set_cmp);
  set_add(heap, ops, blk_r_delim, set_cmp);
  set_add(heap, ops, str_delim, set_cmp);

  word * whitespace = set(heap);
  Object * w = (Object*)whitespace;
  ++(w->refcount);
  set_add(heap, whitespace, space, set_cmp);
  set_add(heap, whitespace, newline, set_cmp);
  set_add(heap, whitespace, tab, set_cmp);
  set_add(heap, whitespace, null, set_cmp);

  word i = 0;
  word * ret = _tokenize(heap, code, code_len, &i, whitespace, ops);
  
  object_delete(heap, ops);
  object_delete(heap, whitespace);

  return ret;
}
