#include <stdint.h>
#include <efi.h>
#include <efilib.h>

#include "config.h"

const word opcode_name_len = 4;
const word reg_des[] = {(word)'r', 0};
const word neg_sgn[] = {(word)'-', 0};

const word bits = sizeof(word) * 8;
const word opcode_len = 8; // How many bits to encode the opcode
const word arg_size = 5; // How many bits to encode a register in an instruction
const word opcode_start = bits - opcode_len;
const word num_regs = (word)1 << arg_size; // Max number of regs that can be used with this encoding
const word inst_mask = (((word)1 << (opcode_len-1))-1) << opcode_start; //leftmost bit tells whether the last arg is a reg or immediate val.
const word sign_bit = ((word)1 << (bits - 1));
const word exc_cont_mask = sign_bit;
const word instr_uses_reg = sign_bit;

const word mx_reg_args = opcode_start / arg_size - 1;

typedef struct operation_data{
  word n_args;
  word name[opcode_name_len];
}op_data;

const word op_data_sz = sizeof(op_data)/sizeof(word);

enum reg_aliases{
  sp = num_regs - 8, 
  fp,
  bp, // pgrm base ptr
  lr, // link reg
  ir, // ivt register
  rr, // result register, used for upper reg when multiplying/dividing.
  pc,
  sr, // status register (exec cont. bit, carry bits, etc)
};

enum opcodes{
  acx, // Atomic CAS
  ads,
  sbs,
  mls,
  mlu,
  dvs,
  dvu,
  and,
  orr,
  xor,
  nor,
  shf, // Bitshift, can do either direction.
  ldr,
  str,
  jnc,
  exc, // Generates interrupt. Can't call it 'int' :/
  // TODO: Add instr for native code exec?

  n_instrs, // Not an instruction, just to tell how many instructions there are.
};

//TODO: prevent immediate instructions where they don't make sense.
//TODO: sign extend arguments on appropriate instructions.
op_data instructions[n_instrs] = {
  [acx]=(op_data){3, {'a', 'c', 'x', '\0'}},
  [ads]=(op_data){3, {'a', 'd', 's', '\0'}},
  [sbs]=(op_data){3, {'s', 'b', 's', '\0'}},
  [mls]=(op_data){3, {'m', 'l', 's', '\0'}},
  [mlu]=(op_data){3, {'m', 'l', 'u', '\0'}},
  [dvs]=(op_data){3, {'d', 'v', 's', '\0'}},
  [dvu]=(op_data){3, {'d', 'v', 'u', '\0'}},
  [and]=(op_data){3, {'a', 'n', 'd', '\0'}},
  [orr]=(op_data){3, {'o', 'r', 'r', '\0'}},
  [xor]=(op_data){3, {'x', 'o', 'r', '\0'}},
  [nor]=(op_data){3, {'n', 'o', 'r', '\0'}},
  [shf]=(op_data){3, {'s', 'h', 'f', '\0'}},
  [ldr]=(op_data){3, {'l', 'd', 'r', '\0'}},
  [str]=(op_data){3, {'s', 't', 'r', '\0'}},
  [jnc]=(op_data){3, {'j', 'n', 'c', '\0'}}, // NOTE: use sr for uncond jump.
  [exc]=(op_data){3, {'e', 'x', 'c', '\0'}}, // ivt addr, int num, int arg
};

typedef struct label_addr{
  word * label;
  word addr;
} lbl_info;

const word lbl_info_sz = sizeof(lbl_info)/sizeof(word);


word get_flx_op_mask(word n){
  return ((word)-1) >> (opcode_len + (n ? (n-1) : 0) * arg_size);
}


word whitespace(word ch){
  return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\0';
}


word ref_eq(word * r1, word * r2, word * extra_params){
  word * s1 = ((lbl_info*)r1)->label;
  word * s2 = ((lbl_info*)r2)->label;
  return s1 == s2;
}


word lbl_streq(word * s1, word * s2, word * max_ptr){
  word * l1 = ((lbl_info*)s1)->label;
  word * l2 = ((lbl_info*)s2)->label;
  for (; l1 < max_ptr && l2 < max_ptr; ++l1, ++l2){
    if (whitespace(*l1)){
      if (!whitespace(*l2)){return 0;}
      return 1;
    }
    if (*l1 != *l2){return 0;}
  }
  return 1;
}


void ignore_whitespace(word * code, word code_sz, word * idx){
  word i = *idx;
  for (; i < code_sz && whitespace(code[i]); ++i){}
  *idx = i;
}


void ignore_non_whitespace(word * code, word code_sz, word * idx){
  word i = *idx;
  for (; i < code_sz && !whitespace(code[i]); ++i){}
  *idx = i;
}


word expect_whitespace(word * code, word code_sz, word * idx){
  word i = *idx;
  if (i >= code_sz || !whitespace(code[i])){return 1;}
  ignore_whitespace(code, code_sz, idx);
  return 0;
}


word expect_whitespace_or_end(word * code, word code_sz, word * idx){
  word ws = expect_whitespace(code, code_sz, idx);
  if (!ws || *idx >= code_sz){return 0;}
  return 1;
}


word match_opcode(word * code, word code_sz, word * index, word * output){
  for (word i = 0; i < n_instrs; ++i){
    word ch;
    word idx = *index;
    for (word j = 0; (ch = instructions[i].name[j]); ++j, ++idx){
      if (idx >= code_sz){return (word)1;}
      if (ch != code[idx]){goto fail;}
    }
    *index = idx;
    *output = i;
    return 0;
  fail:;
  }
  return (word)1;
}


word expect_str(word * code, word code_sz, word * index, const word * str){
  word val;
  word i = 0;
  word idx = *index;
  for (; (val = str[i]); ++i, ++idx){
    if (idx >= code_sz || code[idx] != val){return 1;}
  }
  *index = idx;
  return 0;
}


word expect_hex_from_str(word * code, word code_sz, word * index, word max_val, word * output){
  word res = 0;
  word tmp = 0;
  word idx = *index;
  word i = 0;
  for (; i < (sizeof(word) * 2) && idx < code_sz; ++idx, ++i){
    tmp = code[idx];
    if (tmp >= '0' && tmp <= '9'){
      tmp -= '0';
    }else if (tmp >= 'a' && tmp <= 'f'){
      tmp = tmp - 'a' + 10;
    }else if (tmp >= 'A' && tmp <= 'F'){
      tmp = tmp - 'A' + 10;
    }else{
      break;
    } 

    word pval = (res << 4) | tmp;
    if (pval > max_val){return 1;}
    res = pval;
  }
  // Error if no valid characters read.
  if (!i){return 1;}

  *index = idx;
  *output = res;
  return 0;
}


// code can be treated as just a regular word *, not a special datastructure, although it is an array.
word * compile(word * heap, word * code, word code_sz){
  // Use magic number to estimate of the amount of characters per instr;
  // opc arg arg arg imm\n
  
  if (!code_sz || !heap || !code){return 0;}
  word * arr = array(heap, code_sz / 32, 1);
  if (!arr){return (word*)0;}
  word * lbl_refs = array(heap, code_sz / 8, lbl_info_sz);
  if (!lbl_refs){goto error2;}
  word * lbls = array(heap, code_sz / 8, lbl_info_sz);
  if (!lbls){goto error1;}

  word * tmp_arr;
  word ret_code = 0;
  word output = 0;
  word prgm_ctr = 0;
  word idx = 0;
  ignore_whitespace(code, code_sz, &idx);
  for (; idx < code_sz;){
    // In case of failure parsing, provide a point to go back to.
    word instr_begin = idx;

    // Check if the token is just a number. If so append it to arr.
    ret_code = expect_hex_from_str(code, code_sz, &idx, (word)-1, &output);
    // Check if there is no whitespace after (in which case it could be a label)
    ret_code |= expect_whitespace_or_end(code, code_sz, &idx);
    if (ret_code){
      // In case a hex value was read but there was no whitespace after;
      idx = instr_begin;
      ret_code = match_opcode(code, code_sz, &idx, &output);
      // If an opcode was found but no whitespace comes after, it could be a label.
      ret_code |= expect_whitespace(code, code_sz, &idx);
      if (ret_code){
	idx = instr_begin;
	lbl_info lbl = {code + idx, prgm_ctr};
	if (!array_find(lbls, lbls, (word*)&lbl, ref_eq, (word*)0)){
	  tmp_arr = array_append(heap, lbls, (word*)&lbl);
	  if (!tmp_arr){goto error;}
	  lbls = tmp_arr;
	}
	// Move past the label str.
	ignore_non_whitespace(code, code_sz, &idx);
	// Move past any whitespace after the label str.
	ignore_whitespace(code, code_sz, &idx);
	continue;
      }
    }else{
      tmp_arr = array_append(heap, arr, &output);
      if (!tmp_arr){goto error;}
      arr = tmp_arr;
      ++prgm_ctr;
      continue;
    }

    // Move the opcode into place.
    word instr = output << opcode_start;
    word instr_idx = opcode_start;
    word n_args = instructions[output].n_args;
    // Handle all args except the last one, could be immediate val.
    for (word i = 0; i < (n_args ? (n_args - 1) : 0); ++i){
      ret_code = expect_str(code, code_sz, &idx, reg_des);
      if (ret_code){goto error;}
      ret_code = expect_hex_from_str(code, code_sz, &idx, num_regs - 1, &output);
      if (ret_code){goto error;}
      instr_idx -= arg_size;
      instr |= (output << instr_idx);
      ret_code = expect_whitespace(code, code_sz, &idx);
      if (ret_code){goto error;}
    }

    // Check if there is one or more args required. The last one can either be a reg or an imm val.
    if (n_args){
      // If parsing the last arg fails, provide a place to return to.
      word reset_addr = idx;
      // Detect if there is a negative sign, in case an imm val is given. If it succeeds, the error code is 0.
      word not_neg = expect_str(code, code_sz, &idx, neg_sgn);
      // Try to get a hex value and ensure it is followed by a whitespace. Shift over the mask by 1 to allow one bit for sign.
      ret_code = expect_hex_from_str(code, code_sz, &idx, get_flx_op_mask(n_args) >> 1, &output);
      ret_code |= expect_whitespace_or_end(code, code_sz, &idx);

      // If failure, could be a reg or a label.
      if (ret_code){
	// Go back to the beginning of the last arg of the instr.
	idx = reset_addr;
	// Try to parse as a register.
	ret_code = expect_str(code, code_sz, &idx, reg_des);
        ret_code |= expect_hex_from_str(code, code_sz, &idx, num_regs - 1, &output);
        ret_code |= expect_whitespace_or_end(code, code_sz, &idx);

	// If failure try to parse as a label.
	if (ret_code){
	  // Treat arg as a label. (Don't need to go back, reset_addr is the start of the label str)
	  lbl_info l_info = {code + reset_addr, prgm_ctr};
	  tmp_arr = array_append(heap, lbl_refs, (word*)&l_info);
	  if (!tmp_arr){goto error;}
	  lbl_refs = tmp_arr;
	  // Ignore the rest of the label, if any of it hasn't been gone over yet.
	  ignore_non_whitespace(code, code_sz, &idx);
	}else{
	  // If found to be a register, add it to the instr and modify the instr to show the last arg is a reg.
	  instr_idx -= arg_size;
	  instr |= (output << instr_idx);
	  instr |= instr_uses_reg;
	}
      }else{
	// If a hex value was found, set it to be negative if a negative sign was found before it, then add it to the instr.
	if (!not_neg){
	  output = (~output + 1) & get_flx_op_mask(n_args);
	}
	instr |= output;
      }
    }

    tmp_arr = array_append(heap, arr, &instr);
    if (!tmp_arr){goto error;}
    arr = tmp_arr;
    ignore_whitespace(code, code_sz, &idx);
    ++prgm_ctr;
  }

  // Resolve label refs.
  for (word i = 0; i < array_len(lbl_refs); ++i){
    lbl_info * refs = (lbl_info*)lbl_refs;
    word ref_addr = (refs[i]).addr;
    lbl_info * labels = (lbl_info*)lbls;
    word * ptr = array_find(lbls, lbls, (word*)(refs + i), lbl_streq, code + code_sz);
    if (ptr){
      arr[ref_addr] |= ((lbl_info*)ptr)->addr;
    }else{
      goto error;
    }
  }

  array_delete(heap, lbls);
  array_delete(heap, lbl_refs);
  return arr;

 error:
  array_delete(heap, lbls);
 error1:
  array_delete(heap, lbl_refs);
 error2:
  array_delete(heap, arr);
  return 0;
}


word run(word * bytecode, word * machine){
  // Add 1 so there is a secret register for immediates (to simplify the code)
  word regs[num_regs + 1] = {0};
  regs[ir] = (word)machine;
  regs[sr] = exc_cont_mask;
  regs[pc] = (word)bytecode / sizeof(word);

  word arg_mask = ((word)1 << arg_size) - 1;
  word args[mx_reg_args] = {0};

  while (regs[sr] & exc_cont_mask){
    /* EFI_INPUT_KEY kp; */
    /* EFI_STATUS S = get_char(&kp); */
    /* if (!EFI_ERROR(S)){ */
      
    /* } */
    word instr = *(word*)(regs[pc] * sizeof(word));
    word opcode = (instr & inst_mask) >> opcode_start;
    word n_args = instructions[opcode].n_args;
    word uses_reg = (instr & instr_uses_reg);
    if (!uses_reg && n_args){
      word idx = opcode_len + (n_args - 1) * arg_size;
      word imm = instr & get_flx_op_mask(n_args);
      //Sign extend immediate
      if (imm & (sign_bit >> idx)){
	imm |= ((word)-1) << (bits - idx);
      }
      n_args -= 1;
      // Place immediate in secret register, put idx of the secret register at the end of args.
      regs[num_regs] = imm;
      args[n_args] = num_regs;
    }

    instr >>= bits - (opcode_len + n_args * arg_size);
    for (word i = n_args; i; --i, instr >>= arg_size){
      args[i-1] = instr & arg_mask;
    }

    word x, y;
    switch (opcode){
    case acx:
      regs[args[1]] = atomic_cas((word*)(regs[args[0]]*sizeof(word)), regs[args[1]], regs[args[2]]);
      break;
    case ads:
      regs[args[0]] = regs[args[1]] + regs[args[2]];
      break;
    case sbs:
      regs[args[0]] = regs[args[1]] + (~regs[args[2]] + 1);
      break;
    case mls:
      regs[args[0]] = (word)((sword)regs[args[1]] * (sword)regs[args[2]]);
      break;
    case mlu:
      regs[args[0]] = regs[args[1]] * regs[args[2]];
      break;
    case dvs:
      x = (word)((sword)regs[args[1]] / (sword)regs[args[2]]);
      y = (word)((sword)regs[args[1]] % (sword)regs[args[2]]);
      regs[rr] = y;
      regs[args[0]] = x;
      break;
    case dvu:
      x = regs[args[1]] / regs[args[2]];
      y = regs[args[1]] % regs[args[2]];
      regs[rr] = y;
      regs[args[0]] = x;
      break;
    case and:
      regs[args[0]] = regs[args[1]] & regs[args[2]];
      break;
    case orr:
      regs[args[0]] = regs[args[1]] | regs[args[2]];
      break;
    case xor:
      regs[args[0]] = regs[args[1]] ^ regs[args[2]];
      break;
    case nor:
      regs[args[0]] = ~(regs[args[1]] | regs[args[2]]);
      break;
    case shf:
      sword sh = (sword)(regs[args[2]]);
      word shift = min(abs(sh), bits);
      if (sh < 0){
	regs[args[0]] = regs[args[1]] << shift;
      }else{
	regs[args[0]] = regs[args[1]] >> shift;
      }
      break;
    case ldr:
      regs[args[0]] = *(word*)(sizeof(word)*regs[args[1]] + sizeof(word)*regs[args[2]]);
      break;
    case str:
      *(word*)(sizeof(word)*regs[args[1]] + sizeof(word)*regs[args[2]]) = regs[args[0]];
      break;
    case jnc:
      if (regs[args[0]]){
	regs[pc] = regs[args[1]] + regs[args[2]];
	continue;
      }
      break;
    case exc:
      //regs[rr] = (word)((word*(*)(word*))*(word*)((word*)regs[args[1]] + regs[args[2]]))((word*)regs[args[0]]);
      break;
    default:
      break;
    }

    ++(regs[pc]);
  }
  return regs[rr];
}


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
