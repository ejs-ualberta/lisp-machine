#include "config.h"

void _object_delete(word * heap, word * obj);

word * new_fn(word *heap, word *_sup, word *args, word *exp);
word * get_val(word *obj, word *ns);
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


word * b_plus(word * heap, word * args){
  Object * a_obj = (Object*)args;
  return num_add(heap, (word*)obj_array_idx(args, 1), (word*)obj_array_idx(args, 2));
}


word * b_minus(word * heap, word * args){
  Object * a_obj = (Object*)args;
  word * arg2 = obj_array_idx(args, 2);
  num_negate(arg2);
  return num_add(heap, (word*)obj_array_idx(args, 1), arg2);
}


word * init_machine(word * heap, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE * SystemTable){
  word * handle = (word*)&ImageHandle;
  word * machine = set(heap);

  word st_key[2] = {'s', 't'};
  word * st_str = object(heap, string_type, 2, st_key, 2);
  set_add_str_key(heap, machine, st_str, word_to_num(heap, (word)SystemTable));

  word img_key[3] = {'i', 'm', 'g'};
  word * img_str = object(heap, string_type, 3, img_key, 3);
  set_add_str_key(heap, machine, img_str, word_to_num(heap, (word)&ImageHandle));

  word fb_base_key[4] = {'f', 'b', 'u', 'f'};
  word * fb_base_str = object(heap, string_type, 4, fb_base_key, 4);
  set_add_str_key(heap, machine, fb_base_str, word_to_num(heap, (word)fb_start));

  word fb_hres_key[4] = {'h', 'r', 'e', 's'};
  word * fb_hres_str = object(heap, string_type, 4, fb_hres_key, 4);
  set_add_str_key(heap, machine, fb_hres_str, word_to_num(heap, (word)b_hres));

  word fb_vres_key[4] = {'v', 'r', 'e', 's'};
  word * fb_vres_str = object(heap, string_type, 4, fb_vres_key, 4);
  set_add_str_key(heap, machine, fb_vres_str, word_to_num(heap, (word)b_vres));

  word mem_base_key[3] = {'m', 'e', 'm'};
  word * mem_base_str = object(heap, string_type, 3, mem_base_key, 3);
  set_add_str_key(heap, machine, mem_base_str, word_to_num(heap, (word)heap));

  word mem_sz_key[6] = {'m', 'e', 'm', '_', 's', 'z'};
  word * mem_sz_str = object(heap, string_type, 6, mem_sz_key, 6);
  set_add_str_key(heap, machine, mem_sz_str, word_to_num(heap, global_heap_size));

  word * empty_list = obj_array(heap, 0);
  word b_plus_key[1] = {'+'};
  word * b_plus_str = object(heap, string_type, 1, b_plus_key, 1);
  set_add_str_key(heap, machine, b_plus_str, new_fn(heap, 0, empty_list, word_to_num(heap, (word)b_plus)));

  word b_minus_key[1] = {'-'};
  word * b_minus_str = object(heap, string_type, 1, b_minus_key, 1);
  set_add_str_key(heap, machine, b_minus_str, new_fn(heap, 0, empty_list, word_to_num(heap, (word)b_minus)));

  return machine;
}


void o_del(word * heap, word * obj){
  Object * o = (Object*)obj;
  --((Object*)(o->type))->refcount;
  _object_delete(heap, obj);
}


word * _tokenize(word * heap, word * code, word code_len, word * i, word * ws, word * ops){
  word * ret = obj_array(heap, 8);
  word _tmp[sizeof(Object)+1];
  Object * tmp = (Object*)&_tmp;
  *tmp = (Object){5, 1, (word)string_type, 1, {}};

  while (*i < code_len){
    word * tok = object(heap, string_type, 1, code + *i, 1);
    if (in_set(ws, tok)){
      o_del(heap, tok);
      //--((Object*)string_type)->refcount;
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
	o_del(heap, tok);
	++*i;
	obj_array_append(heap, ret, _tokenize(heap, code, code_len, i, ws, ops));
	continue;
      }else if (!obj_cmp((word*)tok, blk_r_delim)){
	o_del(heap, tok);
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
	o_del(heap, tok);
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
  word fn_sz = 3;
  word fn_init[3] = {(word)ns, (word)args, (word)exp};
  ++((Object*)ns)->refcount;
  ++((Object*)args)->refcount;
  ++((Object*)exp)->refcount;

  word * _self = object(heap, function_type, fn_sz, fn_init, fn_sz);

  if (_sup){set_add_str_key(heap, ns, sup, _sup);}//  --((Object*)_sup)->refcount;}
  set_add_str_key(heap, ns, self, _self);
  //--((Object*)_self)->refcount;
  return _self;
}


word * get_val(word * obj, word * ns){
  word * ret = 0;
  if (!ns){return ret;}
  while (!ret){
    ret = set_get_value(ns, obj);
    Object * s = (Object*)set_get_value(ns, sup);
    if (!s){break;}
    ns = (word*)s->contents[0];
  }
  return ret;
}


word * call_fn(word * heap, word * args, word * ns){
  word * slf = obj_array_idx(args, 0);
  word * exp = (word*)((Object*)slf)->contents[2];
  if (!obj_cmp((word*)((Object*)exp)->type, num_type)){
    word fptr = ((Object*)exp)->contents[0];
    return ((word*(*)(word*, word*))fptr)(heap, args);
  }else if (obj_cmp((word*)((Object*)args)->type, array_type)){
    return get_val(args, ns);
  }

  word * _ns = (word*)((Object*)slf)->contents[0];
  word * arg_names = (word*)((Object*)slf)->contents[1];
  word a_n_len = obj_array_size(arg_names);
  word * ret = 0;

  if (obj_array_size(args) == a_n_len + 1){
    word * _sup = set_get_value(_ns, sup);
    word * tmp_fn = new_fn(heap, _sup, arg_names, exp);
    word * tmp_ns = (word*)((Object*)tmp_fn)->contents[0];
    for (word i = 0; i < a_n_len; ++i){
      set_add_str_key(heap, tmp_ns, obj_array_idx(arg_names, i), obj_array_idx(args, i+1));
    }

    ++((Object*)tmp_fn)->refcount;
    word * result = eval_fn(heap, exp, tmp_ns);
    gc_collect(heap, gc_set);
    --((Object*)tmp_fn)->refcount;
	
    ++((Object*)result)->refcount;
    word len = obj_array_size(result);
    if (len){
      ret = obj_array_idx(result, len - 1);
      set_obj_array_idx(result, len - 1, (word*)0);
    }else{ret = (word*)0;}
    object_delete(heap, result);
    --((Object*)ret)->refcount;
  }
  return ret;
}


word * def_fn(word * heap, word * val, word * ns){
  return new_fn(heap, (word*)set_get_value(ns, self), obj_array_idx(val, 0), obj_array_idx(val, 1));
}


word * name_fn(word * heap, word * val, word * ns){
  word * key = obj_array_idx(val, 0);
  word * res = obj_array_idx(val, 1);
  word * kv_pair = set_remove_str_key(heap, ns, key);
  object_delete(heap, kv_pair);
  set_add_str_key(heap, ns, key, res);
  return res;
}


word * eval_fn(word * heap, word * exp, word * ns){
  word * type = (word*)(((Object*)exp)->type);
  if (obj_cmp(type, array_type)){
    return exp;
  }
  word * tmp = obj_array(heap, 8);
  for (word i = 0; i < obj_array_size(exp); ++i){
    Object * exp_i = (Object*)obj_array_idx(exp, i);
    Queue * op_stack = (Queue*)queue(heap);
    while (set_get_value(ops, (word*)exp_i)){
      queue_push(heap, (word*)op_stack, (word)exp_i);
      exp_i = (Object*)obj_array_idx(exp, ++i);
    }
    
    word * tmp2 = (word*)exp_i;
    Object * op = 0;
    while((op = (Object*)queue_pop(heap, (word*)op_stack))){
      Object * operation = (Object*)set_get_value(ops, (word*)op);
      word * old = tmp2;
      ++((Object*)old)->refcount;
      tmp2 = ((word * (*)(word *, word *, word *))(operation->contents[0]))(heap, tmp2, ns);
      object_delete(heap, old);
    }
    
    obj_array_append(heap, tmp, tmp2);
    free(heap, (word*)op_stack);
  }
  return tmp;
}


word * run_prog(word * heap, word * machine, word * code, word code_sz){
  word _sup[3] = {'s', 'u', 'p'};
  sup = object(heap, string_type, 3, _sup, 3);
  ++((Object*)sup)->refcount;
  word _self[4] = {'s', 'e', 'l', 'f'};
  self = object(heap, string_type, 4, _self, 4);
  ++((Object*)self)->refcount;

  //TODO: make these global constants.
  word _def[1] = {'~'};
  word * def = object(heap, string_type, 1, _def, 1);
  word _call[1] = {':'};
  word * call = object(heap, string_type, 1, _call, 1);
  word _reduce[1] = {'>'};
  word * reduce = object(heap, string_type, 1, _reduce, 1);
  word _name[1] = {'|'};
  word * name = object(heap, string_type, 1, _name, 1);

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

  //word * main = new_fn(heap, (word*)0, empty_lst, exp);
  word fn_init[3] = {(word)machine, (word)empty_lst, (word)empty_lst};
  ++((Object*)machine)->refcount;
  ((Object*)empty_lst)->refcount += 2;
  word * global = object(heap, function_type, 3, fn_init, 3);
  //set_add_str_key(heap, machine, self, global);

  word * main = new_fn(heap, global, empty_lst, exp);

  word * args = obj_array(heap, 1);
  word * expr = obj_array(heap, 1);
  ++((Object*)expr)->refcount;

  obj_array_append(heap, args, (word*)main);
  obj_array_append(heap, expr, call);
  obj_array_append(heap, expr, args);

  word * res = eval_fn(heap, expr, 0);
  ++((Object*)res)->refcount;
  word * ret = obj_array_idx(res, 0);
  set_obj_array_idx(res, 0, 0);
  object_delete(heap, res);

  object_delete(heap, expr); //main is in args
  object_delete(heap, ops);
  gc_collect(heap, gc_set);
  return ret;
}


/* typedef struct stack_frame{ */
/*   Queue * op_stack; */
/*   Object * exp; */
/*   Object * ns; */
/*   Object * tmp_exp; */
/* }StackFrame; */


/* void return_value(Link * last, word *value) { */
/*   if (!last){return;} */
/*   StackFrame * frame = (StackFrame*)last->data; */
/* } */


/* word * run_code(word * heap, word * code, word code_sz){ */
/*   word _sup[3] = {'s', 'u', 'p'}; */
/*   sup = object(heap, string_type, 3, _sup, 3); */
/*   ++((Object*)sup)->refcount; */
/*   word _self[4] = {'s', 'e', 'l', 'f'}; */
/*   self = object(heap, string_type, 4, _self, 4); */
/*   ++((Object*)self)->refcount; */
/*   word _machine[4] = {'m', 'a', 'c', 'h'}; */
/*   word * mach_str = object(heap, string_type, 4, _machine, 4); */
/*   ++((Object*)mach_str)->refcount; */

/*   //TODO: make these global constants. */
/*   word _def[1] = {'~'}; */
/*   def = object(heap, string_type, 1, _def, 1); */
/*   word _call[1] = {':'}; */
/*   call = object(heap, string_type, 1, _call, 1); */
/*   word _reduce[1] = {'>'}; */
/*   reduce = object(heap, string_type, 1, _reduce, 1); */
/*   word _name[1] = {'|'}; */
/*   name = object(heap, string_type, 1, _name, 1); */

/*   ops = set(heap); */
/*   Object * o = (Object*)ops; */
/*   ++(o->refcount); */

/*   set_add(heap, ops, def, &obj_cmp); */
/*   set_add(heap, ops, call, &obj_cmp); */
/*   set_add(heap, ops, reduce, &obj_cmp); */
/*   set_add(heap, ops, name, &obj_cmp); */
/*   word * exp_main = tokenize(heap, code, code_sz); */
  
/*   word * stack = queue(heap); */
/*   word * op_stack = queue(heap); */

/*   queue_push(heap, op_stack, (word)call); */
/*   word * args = obj_array(heap, 1); */
/*   obj_array_append(heap, args, new_fn(heap, 0, obj_array(heap, 0), exp_main)); */
/*   StackFrame * frame = (StackFrame*)alloc(heap, sizeof(StackFrame)/sizeof(word)); */
/*   *frame = (StackFrame){(Queue*)op_stack, (Object*)args, 0}; */
/*   queue_push(heap, stack, (word)frame); */

/*   StackFrame * current; */
/*   word * ret_val; */
/*   while((current = (StackFrame*)((Link*)((Queue*)stack)->last)->data)){ */
/*     Queue * op_stack = current->op_stack; */
/*     Object * tmp_exp = current->tmp_exp; */
/*     Object * op = (Object*)queue_pop(heap, (word*)op_stack); */
/*     if (!op){ */
/*       ret_val = (word*)current->tmp_exp; */
/*       StackFrame * frame = (StackFrame*)queue_last(heap, (word*)stack); */
/*       free(heap, (word*)frame->op_stack); */
/*       return_value((Link*)((Queue*)stack)->last, ret_val); */
/*       continue; */
/*     } */

/*     switch(op->contents[0]){ */
/*     case '|': */
      
/*       break; */
/*     case '~': */
/*       break; */
/*     case '>': */
/*       if (!obj_cmp(tmp_exp, array_type)){ */
	
/*       } */
/*       break; */
/*     case ':': */
/*       break; */
/*     } */
/*   } */

/*   return ret_val; */
/* } */

/* |>[pair ~>[[v1 v2][~>[[cond][:>[:cond :v1 :v2]]]]]] */
/* |>[#T ~>[[v1 v2][:v1]]] */
/* |>[#F ~>[[v1 v2][:v2]]] */
/* |>[if ~>[[cond exp1 exp2][:>[:>[:pair :exp1 :exp2] :cond]]]] */
/* :>[:>[:if :#F :#F :#T] True False] */
