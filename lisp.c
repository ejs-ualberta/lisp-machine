#include "config.h"

void _object_delete(word * heap, word * obj);

word * new_fn(word *heap, word *_sup, word *args, word *exp);
word * get_val(word *obj, word * self);
word * call_fn(word * heap, word * val, word * self);
word * def_fn(word * heap, word * val, word * self);
word * name_fn(word * heap, word * val, word * self);
word * eval_fn(word * heap, word * exp, word * self);
enum function {f_ns = 0, f_args, f_exp};

word * sup = 0;
word * slf = 0;

word * def = (word*)0;
word * call = (word*)0;
word * reduce = (word*)0;
word * name = (word*)0;
word * comp = (word*)0;
word * blk_l_delim = (word*)0;
word * blk_r_delim = (word*)0;
word * str_delim = (word*)0;

word * space = (word*)0;
word * newline = (word*)0;
word * tab = (word *)0;
word * null = (word *)0;

word * ops;


/* word * init_machine(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE * SystemTable){ */
/*   word * handle = (word*)&ImageHandle; */
/*   word * machine = set(global_heap_start); */
/*   ((Object*)machine)->refcount += 1; */

/*   word st_key[7] = {'s', 'y', 's', '_', 't', 'a', 'b'}; */
/*   word * st_str = object(global_heap_start, string_type, 7, st_key, 7); */
/*   word * st_val = object(global_heap_start, num_type, 1, (word*)SystemTable, 1); */
/*   set_add_str_key(global_heap_start, machine, st_str, st_val); */

/*   word img_key[3] = {'i', 'm', 'g'}; */
/*   word * img_str = object(global_heap_start, string_type, 3, img_key, 3); */
/*   word * img_val = object(global_heap_start, num_type, 1, (word*)&handle, 1); */
/*   set_add_str_key(global_heap_start, machine, img_str, img_val); */

/*   /\* word types_key[5] = {'t', 'y', 'p', 'e', 's'}; *\/ */
/*   /\* word * types_str = object(global_heap_start, string_type, 5, types_key, 5); *\/ */
/*   /\* word * types_set = set(global_heap_start); *\/ */
/*   /\* set_add_str_key(global_heap_start, types_set, string_type, string_type); *\/ */
/*   /\* set_add_str_key(global_heap_start, types_set, num_type, num_type); *\/ */
/*   /\* set_add_str_key(global_heap_start, types_set, array_type, array_type); *\/ */
/*   /\* set_add_str_key(global_heap_start, types_set, set_type, set_type); *\/ */
/*   /\* set_add_str_key(global_heap_start, types_set, function_type, function_type); *\/ */
/*   /\* set_add_str_key(global_heap_start, types_set, cell_type, cell_type); *\/ */
/*   /\* /\\* set_add_str_key(global_heap_start, types_set, native_type, native_type); *\\/ *\/ */
/*   /\* set_add_str_key(global_heap_start, machine, types_str, types_set); *\/ */

/*   word pc_key[2] = {'p', 'c'}; */
/*   word * pc_str = object(global_heap_start, string_type, 2, pc_key, 2); */
/*   word * pc_set = set(global_heap_start); */
/*   set_add_str_key(global_heap_start, machine, pc_str, pc_set); */

/*   word fb_set_key[2] = {'f', 'b'}; */
/*   word * fb_set_str = object(global_heap_start, string_type, 2, fb_set_key, 2); */
/*   word * fb_set = set(global_heap_start); */
/*   set_add_str_key(global_heap_start, pc_set, fb_set_str, fb_set); */
/*   word fb_base_key[4] = {'b', 'a', 's', 'e'}; */
/*   word * fb_base_str = object(global_heap_start, string_type, 4, fb_base_key, 4); */
/*   word * fb_base_val = object(global_heap_start, num_type, 1, (word*)&fb_start, 1); */
/*   word fb_hres_key[4] = {'h', 'r', 'e', 's'}; */
/*   word * fb_hres_str = object(global_heap_start, string_type, 4, fb_hres_key, 4); */
/*   word * fb_hres_val = object(global_heap_start, num_type, 1, (word*)&b_hres, 1); */
/*   word fb_vres_key[4] = {'v', 'r', 'e', 's'}; */
/*   word * fb_vres_str = object(global_heap_start, string_type, 4, fb_vres_key, 4); */
/*   word * fb_vres_val = object(global_heap_start, num_type, 1, (word*)&b_vres, 1); */
/*   set_add_str_key(global_heap_start, fb_set, fb_base_str, fb_base_val); */
/*   set_add_str_key(global_heap_start, fb_set, fb_hres_str, fb_hres_val); */
/*   set_add_str_key(global_heap_start, fb_set, fb_vres_str, fb_vres_val); */

/*   word mem_key[3] = {'m', 'e', 'm'}; */
/*   word * mem_set_str = object(global_heap_start, string_type, 3, mem_key, 3); */
/*   word * mem_set = set(global_heap_start); */
/*   set_add_str_key(global_heap_start, pc_set, mem_set_str, mem_set); */
/*   word * mem_base = object(global_heap_start, num_type, 1, (word*)&global_heap_start, 1); */
/*   set_add_str_key(global_heap_start, mem_set, fb_base_str, mem_base); */
/*   word mem_sz_key[3] = {'s', 'z'}; */
/*   word * mem_sz_str = object(global_heap_start, string_type, 2, mem_sz_key, 2); */
/*   word * mem_sz = object(global_heap_start, num_type, 1, (word*)&global_heap_size, 1); */
/*   set_add_str_key(global_heap_start, mem_set, mem_sz_str, mem_sz); */

/*   return machine; */
/* } */


word * _tokenize(word * heap, word * code, word code_len, word * i, word * ws, word * ops){
  word * ret = obj_array(heap, 8);
  word tmp_init = 0;
  Object * tmp = (Object*)object(heap, string_type, 1, &tmp_init, 1);
  ++tmp->refcount;

  word * tok;
  while (*i < code_len){
    tok = object(heap, string_type, 1, code + *i, 1);
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
	obj_array_append(heap, ret, tok);
      }else if (!obj_cmp((word*)tok, blk_l_delim)){
	o_del(heap, tok);
	++*i;
	obj_array_append(heap, ret, _tokenize(heap, code, code_len, i, ws, ops));
	continue;
      }else if (!obj_cmp((word*)tok, blk_r_delim)){
	object_delete(heap, (word*)tmp);
	o_del(heap, tok);
	++*i;
	return ret;
      }else{
	obj_array_append(heap, ret, in_set(ops, tok));
	o_del(heap, tok);
      }
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
  object_delete(heap, (word*)tmp);
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
  word _comp[1] = {'<'};
  comp = object(heap, string_type, 1, _comp, 1);
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
  set_add(heap, ops, comp, set_cmp);
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
  word * _self = object(heap, function_type, fn_sz, 0, 0);
  if (!_self){return 0;}
  if (gc_add(heap, _self)){
    object_delete(heap, _self);
    return 0;
  }
  ((Object*)_self)->size = fn_sz;
  ((Object*)_self)->contents[f_ns] = (word)ns;
  ((Object*)_self)->contents[f_args] = (word)args;
  ((Object*)_self)->contents[f_exp] = (word)exp;
  ++((Object*)ns)->refcount;
  ++((Object*)args)->refcount;
  ++((Object*)exp)->refcount;

  if (_sup){set_add_str_key(heap, ns, sup, _sup);}//  --((Object*)_sup)->refcount;}
  /* set_add_str_key(heap, ns, self, _self); */
  /* //--((Object*)_self)->refcount; */
  return _self;
}


void del_fn(word * heap, word * fn){
  word * ns = (word*)((Object*)fn)->contents[f_ns];
  Object * kv_pair = (Object*)set_remove_str_key(heap, ns, sup);
  word * _sup = (word*)kv_pair->contents[1];
  kv_pair->contents[1] = 0;
  object_delete(heap, (word*)kv_pair);
  --((Object*)_sup)->refcount;
  gc_del_obj(heap, fn);
}


word * get_val(word * obj, word * self){
  word * ns = (word*)((Object*)self)->contents[0];
  word * ret = 0;
  if (!ns){return ret;}
  while (!ret){
    ret = set_get_value(ns, obj);
    Object * s = (Object*)set_get_value(ns, sup);
    if (!s){break;}
    ns = (word*)s->contents[0];
  }
  if (!ret && !obj_cmp(obj, slf)){
    return self;
  }
  return ret;
}


word * call_fn(word * heap, word * args, word * self){
  if (obj_cmp((word*)((Object*)args)->type, array_type)){
    return get_val(args, self);
  }
  word * _self = obj_array_idx(args, 0);
  word * _ns = (word*)((Object*)_self)->contents[0];
  word * arg_names = (word*)((Object*)_self)->contents[1];
  word * exp = (word*)((Object*)_self)->contents[2];
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
    word * result = eval_fn(heap, exp, tmp_fn);
    ++((Object*)result)->refcount;
    check_heap_capacity(heap);
    --((Object*)tmp_fn)->refcount;
	
    word len = obj_array_size(result);
    if (len){
      ret = obj_array_idx(result, len - 1);
      set_obj_array_idx(result, len - 1, (word*)0);
      --((Object*)ret)->refcount;
    }else{ret = (word*)0;}
    object_delete(heap, result);
  }
  return ret;
}


word * def_fn(word * heap, word * val, word * self){
  return new_fn(heap, self, obj_array_idx(val, 0), obj_array_idx(val, 1));
}


word * name_fn(word * heap, word * val, word * self){
  word * ns = (word*)((Object*)self)->contents[0];
  word * key = obj_array_idx(val, 0);
  word * res = obj_array_idx(val, 1);
  word * kv_pair = set_remove_str_key(heap, ns, key);
  object_delete(heap, kv_pair);
  set_add_str_key(heap, ns, key, res);
  return res;
}


word * eval_fn(word * heap, word * exp, word * self){
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
      tmp2 = ((word * (*)(word *, word *, word *))(operation->contents[0]))(heap, tmp2, self);
      object_delete(heap, old);
    }
    
    obj_array_append(heap, tmp, tmp2);
    free(heap, (word*)op_stack);
  }
  return tmp;
}


word * run_prog(word * heap, word * machine, word * code, word code_sz){
  sup = cstr_to_string(heap, "sup");
  ++((Object*)sup)->refcount;
  slf = cstr_to_string(heap, "self");
  ++((Object*)slf)->refcount;

  //TODO: make these global constants.
  word * def = cstr_to_string(heap, "~");
  word * call = cstr_to_string(heap, ":");
  word * reduce = cstr_to_string(heap, ">");
  word * name = cstr_to_string(heap, "|");

  ops = set(heap);
  Object * o = (Object*)ops;
  ++(o->refcount);

  word * (*df)(word *, word *, word *) = def_fn;
  word * (*cf)(word *, word *, word *) = call_fn;
  word * (*ef)(word *, word *, word *) = eval_fn;
  word * (*nf)(word *, word *, word *) = name_fn;
  set_add_str_key(heap, ops, def, word_to_num(heap, (word)df));
  set_add_str_key(heap, ops, call, word_to_num(heap, (word)cf));
  set_add_str_key(heap, ops, reduce, word_to_num(heap, (word)ef));
  set_add_str_key(heap, ops, name, word_to_num(heap, (word)nf));
  word * exp = tokenize(heap, code, code_sz);
  word * empty_lst = obj_array(heap, 0);
  word * main = new_fn(heap, (word*)0, empty_lst, exp);
  word * args = obj_array(heap, 1);
  word * expr = obj_array(heap, 1);
  ++((Object*)expr)->refcount;

  obj_array_append(heap, args, (word*)main);
  obj_array_append(heap, expr, call);
  obj_array_append(heap, expr, args);

  word * res = eval_fn(heap, expr, main);
  ++((Object*)res)->refcount;
  word * ret = obj_array_idx(res, 0);
  set_obj_array_idx(res, 0, 0);
  object_delete(heap, res);

  object_delete(heap, expr); //main is in args
  object_delete(heap, ops);
  gc_collect(heap);
  return ret;
}


typedef struct stack_frame{
  Object * self;
  word * exp;
  word * tmp;
  word * op_stk;
  word * old;
  word i;
  word state;
  struct stack_frame * prev_frame;
} StackFrame;


StackFrame * get_frame(word * heap, word * self, word * exp, word state, word * prev_frame){
  StackFrame * frame = (StackFrame*)alloc(heap, sizeof(StackFrame)/sizeof(word));
  if (!frame){
    return frame;
  }
  // TODO: increase refcs? 
  frame->self = (Object*)self;
  frame->exp = exp;
  frame->tmp = 0;
  frame->op_stk = 0;
  frame->old = 0;
  frame->i = 0;
  frame->state = state;
  frame->prev_frame = (StackFrame*)prev_frame;
  return frame;
}


StackFrame * del_frame(word *heap, StackFrame *frame) {
  StackFrame * prev = frame->prev_frame;
  free(heap, (word*)frame);
  return prev;
}


enum state {cf, cf_ret, df, ef, ef_0, ef_1, ccf, nf};
word * _run_prog(word * heap, word * exception_fifo, word * main_expr, word * ops){
  word * regs = init_regs(heap, 0);
  word * ret = 0;
  word * prev = cstr_to_string(heap, "tmp_obj");
  word * self = obj_array_idx(main_expr, 0);
  StackFrame * frame = get_frame(heap, self, main_expr, cf, 0);
  word * op = 0;
  while(frame){
    //uart_puts("\nF ");uart_print_uint(frame, 16);uart_puts("\n");
    switch (frame->state){
    case cf:;
      if (obj_cmp((word*)(((Object*)frame->exp)->type), array_type)){
	ret =  get_val(frame->exp, (word*)frame->self);
	break;
      }
      word * _self = obj_array_idx(frame->exp, 0);
      if (!obj_cmp((word*)((Object*)_self)->type, bcode_type)){
	ret = run_expr(exception_fifo, regs, frame->exp);
	break;
      }
      word * _ns = (word*)((Object*)_self)->contents[f_ns];
      word * arg_names = (word*)((Object*)_self)->contents[f_args];
      word * exp = (word*)((Object*)_self)->contents[f_exp];
      word a_n_len = obj_array_size(arg_names);
      if (obj_array_size(frame->exp) == a_n_len + 1){
	word * _sup = set_get_value(_ns, sup);
	word * tmp_fn = new_fn(heap, _sup, arg_names, exp);
	word * tmp_ns = (word*)((Object*)tmp_fn)->contents[0];
	for (word i = 0; i < a_n_len; ++i){
	  set_add_str_key(heap, tmp_ns, obj_array_idx(arg_names, i), obj_array_idx(frame->exp, i+1));
	}

	++((Object*)tmp_fn)->refcount;
	frame->state = cf_ret;
	frame->tmp = (word*)tmp_fn;
	frame = get_frame(heap, tmp_fn, exp, ef, (word*)frame);
	continue;
      }
      ret = 0;
      break;
    case cf_ret:
      ++((Object*)ret)->refcount; // ret is still result
      word * result = ret;
      del_fn(heap, prev);
      check_heap_capacity(heap);
      prev = frame->tmp;
      --((Object*)frame->tmp)->refcount;
      word len = obj_array_size(result);
      if (len){
	ret = obj_array_idx(result, len - 1);
	set_obj_array_idx(result, len - 1, (word*)0);
	--((Object*)ret)->refcount;
      }else{ret = (word*)0;}
      object_delete(heap, result);
      break;
    case df:
      ret = new_fn(heap, (word*)frame->self, obj_array_idx(frame->exp, 0), obj_array_idx(frame->exp, 1));
      break;
    case ef:
      if (obj_cmp((word*)(((Object*)frame->exp)->type), array_type) || frame->i >= obj_array_size(frame->exp)){
	break;
      }
      frame->tmp = obj_array(heap, 8);
      frame->op_stk = queue(heap);
    case ef_0:
      ret = obj_array_idx(frame->exp, frame->i);
      while (set_get_value(ops, ret)){
	queue_push(heap, frame->op_stk, (word)ret);
	ret = obj_array_idx(frame->exp, ++frame->i);
      }
      frame->state = ef_1;
      frame->old = 0;
    case ef_1:
      object_delete(heap, frame->old);
      if ((op = (word*)queue_pop(heap, frame->op_stk))){
	frame->old = ret;
	++((Object*)frame->old)->refcount;
	frame = get_frame(heap, (word*)frame->self, (word*)ret, ((Object*)set_get_value(ops, op))->contents[0], (word*)frame);
	continue;
      }else{
	obj_array_append(heap, frame->tmp, ret);
	++frame->i;
	if (frame->i >= obj_array_size(frame->exp)){
	  free(heap, frame->op_stk);
	  ret = frame->tmp;
	  break;
	}
	frame->state = ef_0;
	continue;
      }
    case ccf:
      ret = comp_expr(heap, frame->exp);
      break;
    case nf:;
      word * ns = (word*)frame->self->contents[f_ns];
      word * key = obj_array_idx(frame->exp, 0);
      word * kv_pair = set_remove_str_key(heap, ns, key);
      object_delete(heap, kv_pair);
      ret = obj_array_idx(frame->exp, 1);
      set_add_str_key(heap, ns, key, ret);
      break;
    }
    //rec_obj_print(ret);uart_puts("\n");
    frame = del_frame(heap, frame);
  }
  free(heap, regs);
  object_delete(heap, main_expr);
  return ret;
}


word * run_prog_stack(word * heap, word * exception_fifo, word * code, word code_sz){
  sup = cstr_to_string(heap, "sup");
  ++((Object*)sup)->refcount;
  slf = cstr_to_string(heap, "self");
  ++((Object*)slf)->refcount;

  word stack_sz = 1024;
  word * stack = alloc(heap, stack_sz);
  if (!stack){
    uart_puts("Not enough memory for stack.");
    return 0;
  }

  //TODO: make these global constants.
  word * def = cstr_to_string(heap, "~");
  word * call = cstr_to_string(heap, ":");
  word * reduce = cstr_to_string(heap, ">");
  word * comp = cstr_to_string(heap, "<");
  word * name = cstr_to_string(heap, "|");

  ops = set(heap);
  Object * o = (Object*)ops;
  ++(o->refcount);

  set_add_str_key(heap, ops, def, word_to_num(heap, (word)df));
  set_add_str_key(heap, ops, call, word_to_num(heap, (word)cf));
  set_add_str_key(heap, ops, reduce, word_to_num(heap, (word)ef));
  set_add_str_key(heap, ops, comp, word_to_num(heap, (word)ccf));
  set_add_str_key(heap, ops, name, word_to_num(heap, (word)nf));

  word * exp = tokenize(heap, code, code_sz);
  word * arg_lst = obj_array(heap, 8);
  obj_array_append(heap, arg_lst, cstr_to_string(heap, "STACK"));
  obj_array_append(heap, arg_lst, cstr_to_string(heap, "HEAP"));
  word * main = new_fn(heap, (word*)0, arg_lst, exp);
  word * expr = obj_array(heap, 8);
  obj_array_append(heap, expr, main);
  obj_array_append(heap, expr, word_to_num(heap, ((word)stack)/sizeof(word)));
  obj_array_append(heap, expr, word_to_num(heap, ((word)heap)/sizeof(word)));

  ++((Object*)expr)->refcount;
  word * ret = _run_prog(heap, exception_fifo, expr, ops);
  ++((Object*)ret)->refcount;
  free(heap, stack);
  object_delete(heap, expr);
  object_delete(heap, ops);
  gc_collect(heap);
  --((Object*)ret)->refcount;
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
