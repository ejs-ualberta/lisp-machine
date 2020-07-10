#include "config.h"

const word opcode_name_len = 4;
const word bits = sizeof(word) * 8;
const word reg_des[] = {(word)'r', 0};
const word neg_sgn[] = {(word)'-', 0};
const word opcode_len = 8; // How many bits to encode the opcode
const word arg_size = 5; // How many bits to encode a register in an instruction
const word opcode_start = bits - opcode_len;
const word mx_reg_args = opcode_start / arg_size - 1;
const word num_regs = (word)1 << arg_size; // Max number of regs that can be used with this encoding
const word inst_mask = (((word)1 << (opcode_len-1))-1) << (bits - opcode_len); //leftmost bit tells whether the last arg is a reg or immediate val.
const word instr_uses_reg = ((word)1 << (bits - 1));

typedef struct operation_data{
  word n_args;
  word name[opcode_name_len];
}op_data;

const word op_data_sz = sizeof(op_data)/sizeof(word);

enum reg_aliases{
  zr = num_regs - 9,
  rr, // result register, used for upper reg when multiplying/dividing.
  sp,
  fp,
  bp, // pgrm base ptr
  lr, // link reg
  pc,
  ir, // ivt register
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
  return ((word)-1) >> (opcode_len + n * arg_size);
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

  word * lbl_refs = array(heap, 16, lbl_info_sz);
  if (!lbl_refs){goto error2;}

  word * lbls = array(heap, 16, lbl_info_sz);
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
      ret_code = expect_hex_from_str(code, code_sz, &idx, num_regs, &output);
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
      // Try to get a hex value and ensure it is followed by a whitespace.
      ret_code = expect_hex_from_str(code, code_sz, &idx, get_flx_op_mask(n_args), &output);
      ret_code |= expect_whitespace_or_end(code, code_sz, &idx);

      // If failure, could be a reg or a label.
      if (ret_code){
	// Go back to the beginning of the last arg of the instr.
	idx = reset_addr;
	// Try to parse as a register.
	ret_code = expect_str(code, code_sz, &idx, reg_des);
        ret_code |= expect_hex_from_str(code, code_sz, &idx, num_regs, &output);
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


void run(word bytecode, word bc_sz){
  
}
