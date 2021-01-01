#include "config.h"

void _object_delete(word * heap, word * obj);

word * new_fn(word *heap, word *_sup, word *args, word *exp);
word * fn_call(word * heap, word * slf, word * args);
word * get_val(word *obj, word *ns);
word * dispatch(word *heap, word * op, word * exp, word * ns, word * i);
word * call_fn(word * heap, word * val, word * ns);
word * def_fn(word * heap, word * val, word * ns);
word * name_fn(word * heap, word * val, word * ns);
word * eval_fn(word * heap, word * exp, word * ns);

word * sup = 0;
word * self = 0;

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
word * null = (word *)0;

word * ops;


word * init_machine(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE * SystemTable){
  word * handle = (word*)&ImageHandle;
  word * machine = set(global_heap_start);
  ((Object*)machine)->refcount += 1;

  word st_key[7] = {'s', 'y', 's', '_', 't', 'a', 'b'};
  word * st_str = object(global_heap_start, string_type, 7, st_key, 7);
  word * st_val = object(global_heap_start, num_type, 1, (word*)SystemTable, 1);
  set_add_str_key(global_heap_start, machine, st_str, st_val);

  word img_key[3] = {'i', 'm', 'g'};
  word * img_str = object(global_heap_start, string_type, 3, img_key, 3);
  word * img_val = object(global_heap_start, num_type, 1, (word*)&handle, 1);
  set_add_str_key(global_heap_start, machine, img_str, img_val);

  /* word types_key[5] = {'t', 'y', 'p', 'e', 's'}; */
  /* word * types_str = object(global_heap_start, string_type, 5, types_key, 5); */
  /* word * types_set = set(global_heap_start); */
  /* set_add_str_key(global_heap_start, types_set, string_type, string_type); */
  /* set_add_str_key(global_heap_start, types_set, num_type, num_type); */
  /* set_add_str_key(global_heap_start, types_set, array_type, array_type); */
  /* set_add_str_key(global_heap_start, types_set, set_type, set_type); */
  /* set_add_str_key(global_heap_start, types_set, function_type, function_type); */
  /* set_add_str_key(global_heap_start, types_set, cell_type, cell_type); */
  /* /\* set_add_str_key(global_heap_start, types_set, native_type, native_type); *\/ */
  /* set_add_str_key(global_heap_start, machine, types_str, types_set); */

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


word * new_fn(word * heap, word * _sup, word * args, word * exp){
  word * ns = set(heap);
  word fn_init[3] = {(word)ns, (word)args, (word)exp};
  ++((Object*)ns)->refcount;
  ++((Object*)args)->refcount;
  ++((Object*)exp)->refcount;
  word * _self = object(heap, function_type, 3, fn_init, 3);
  if (_sup){set_add_str_key(heap, ns, sup, _sup);}//  --((Object*)_sup)->refcount;}
  set_add_str_key(heap, ns, self, _self);
  //--((Object*)_self)->refcount;
  return _self;
}


word * fn_call(word * heap, word * slf, word * args){
  word * ns = (word*)((Object*)slf)->contents[0];
  word * arg_names = (word*)((Object*)slf)->contents[1];
  word * exp = (word*)((Object*)slf)->contents[2];
  word a_n_len = obj_array_size(arg_names);
  word * ret = 0;
  if (obj_array_size(args) == a_n_len + 1){
    word * tmp_fn = new_fn(heap, set_get_value(ns, sup), arg_names, exp);
    ++((Object*)tmp_fn)->refcount;
    word * tmp_ns = (word*)((Object*)tmp_fn)->contents[0];
    for (word i = 0; i < a_n_len; ++i){
      set_add_str_key(heap, tmp_ns, obj_array_idx(arg_names, i), obj_array_idx(args, i+1));
    }
    word * result = eval_fn(heap, exp, tmp_ns);
    ++((Object*)result)->refcount;
    word len = obj_array_size(result);
    if (len){
      ret = obj_array_idx(result, len - 1);
      set_obj_array_idx(result, len - 1, (word*)0);
    }else{ret = (word*)0;}
    object_delete(heap, result);
    object_delete(heap, tmp_fn);
  }
  return ret;
}


word * get_val(word * obj, word * ns){
  word * ret = 0;
  while (ns && !ret){
    ret = set_get_value(ns, obj);
    ns = (word*)((Object*)set_get_value(ns, sup))->contents[0];
  }
  return ret;
}


word * dispatch(word * heap, word * op, word * exp, word * ns, word * i){
  ++*i;
  if (*i < obj_array_size(exp)){
    Object * exp_i = (Object*)obj_array_idx(exp, *i);
    word * val = 0;
    if (!obj_cmp((word*)(exp_i->type), string_type) && set_get_value(ops, (word*)exp_i)){
      val = dispatch(heap, (word*)exp_i, exp, ns, i);
    }else{
      val = (word*)exp_i;
    }
    Object * operation = (Object*)set_get_value(ops, op);
    word * result = ((word * (*)(word *, word *, word *))(operation->contents[0]))(heap, val, ns);
    return result;
  }
  return 0;
}


word * call_fn(word * heap, word * val, word * ns){
  if (!obj_cmp((word*)((Object*)val)->type, array_type)){
    word * obj = obj_array_idx(val, 0);
    return fn_call(heap, obj, val);
  }else{
    word * res = get_val(val, ns);
    return res;
  }
}


word * def_fn(word * heap, word * val, word * ns){
  return new_fn(heap, (word*)set_get_value(ns, self), obj_array_idx(val, 0), obj_array_idx(val, 1));
}


word * name_fn(word * heap, word * val, word * ns){
  word * res = obj_array_idx(val, 1);
  set_add_str_key(heap, ns, obj_array_idx(val, 0), res);
  return res;
}


word * eval_fn(word * heap, word * exp, word * ns){
  word * type = (word*)(((Object*)exp)->type);
  if (!obj_cmp(type, array_type)){
    word * tmp = obj_array(heap, 8);
    for (word i = 0; i < obj_array_size(exp); ++i){
      Object * exp_i = (Object*)obj_array_idx(exp, i);
      if (!obj_cmp((word*)(exp_i->type), string_type) && set_get_value(ops, (word*)exp_i)){
	word * tmp2 = dispatch(heap, (word*)exp_i, exp, ns, &i);
	obj_array_append(heap, tmp, tmp2);
      }else{
	obj_array_append(heap, tmp, (word*)exp_i);
      }
    }
    return tmp;
  }else{
    return exp;
  }
}


word * run_prog(word * heap, word * machine, word * code, word code_sz){
  word _sup[3] = {'s', 'u', 'p'};
  sup = object(heap, string_type, 3, _sup, 3);
  word _self[4] = {'s', 'e', 'l', 'f'};
  self = object(heap, string_type, 4, _self, 4);
  word _machine[4] = {'m', 'a', 'c', 'h'};
  word * mach_str = object(heap, string_type, 4, _machine, 4);

  //TODO: make these global constants.
  word _def[1] = {'~'};
  def = object(heap, string_type, 1, _def, 1);
  word _call[1] = {':'};
  call = object(heap, string_type, 1, _call, 1);
  word _reduce[1] = {'>'};
  reduce = object(heap, string_type, 1, _reduce, 1);
  word _name[1] = {'|'};
  name = object(heap, string_type, 1, _name, 1);

  ops = set(heap);
  Object * o = (Object*)ops;
  ++(o->refcount);

  word * (*df)(word *, word *, word *) = def_fn;
  word * (*cf)(word *, word *, word *) = call_fn;
  word * (*ef)(word *, word *, word *) = eval_fn;
  word * (*nf)(word *, word *, word *) = name_fn;
  set_add_str_key(heap, ops, def, object(heap, num_type, 1, (word*)&df, 1));
  set_add_str_key(heap, ops, call, object(heap, num_type, 1, (word*)&cf, 1));
  set_add_str_key(heap, ops, reduce, object(heap, num_type, 1, (word*)&ef, 1));
  set_add_str_key(heap, ops, name, object(heap, num_type, 1, (word*)&nf, 1));
  word * exp = tokenize(heap, code, code_sz);
  
  word * empty_lst = obj_array(heap, 0);
  word * main = new_fn(heap, (word*)0, empty_lst, exp);
  word * args = obj_array(heap, 1);
  ++((Object*)args)->refcount;
  obj_array_append(heap, args, (word*)main);

  word * res = fn_call(heap, main, args);
  object_delete(heap, args); //main is in args
  object_delete(heap, ops);
  return res;
}