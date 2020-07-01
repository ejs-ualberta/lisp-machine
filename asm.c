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
const word opcode_uses_reg = ((word)1 << (bits - 1));

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
  ace, // Atomic CAS
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
  [ace]=(op_data){3, {'a', 'c', 'e', '\0'}},
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
    }else if (tmp >= 'a' && tmp <= 'z'){
      tmp = tmp - 'a' + 10;
    }else if (tmp >= 'A' && tmp <= 'Z'){
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


// code is just a regular word *, not a special datastructure. 
word * compile(word * heap, word * code, word code_sz){
  // Use magic number to estimate of the amount of characters per instr;
  // opc arg arg arg imm\n
  word * tmp_arr;
  word * arr = array(heap, code_sz / 32, 1);
  if (!arr){return (word*)0;}

  word * lbl_refs = array(heap, 16, lbl_info_sz);
  if (!lbl_refs){
    array_delete(heap, arr);
    return (word*)0;
  }

  word prgm_ctr = 0;
  word ret_code = 0;
  word output = 0;
  word idx = 0;
  ignore_whitespace(code, code_sz, &idx);
  for (; idx < code_sz; ++prgm_ctr){
    word instr_begin = idx;

    ret_code = match_opcode(code, code_sz, &idx, &output);
    if (ret_code){goto is_label;}
    ret_code = expect_whitespace(code, code_sz, &idx);
    if (ret_code){goto is_label;}

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
    if (n_args){
      word not_neg = expect_str(code, code_sz, &idx, neg_sgn);
      ret_code = expect_hex_from_str(code, code_sz, &idx, get_flx_op_mask(n_args), &output);
      if (ret_code){
	if (!not_neg){idx -= strlen(neg_sgn);}
	ret_code = expect_str(code, code_sz, &idx, reg_des);
	ret_code |= expect_hex_from_str(code, code_sz, &idx, num_regs, &output);
	if (ret_code){
	  idx -= strlen(reg_des);
	  lbl_info l_info = {code + idx, prgm_ctr};
	  tmp_arr = array_append(heap, lbl_refs, (word*)&l_info);
	  if (!tmp_arr){goto error;}
	  lbl_refs = tmp_arr;
	  ignore_non_whitespace(code, code_sz, &idx);
	}else{
	  instr_idx -= arg_size;
	  instr |= (output << instr_idx);
	}
      }else{
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
    continue;

  is_label:
    idx = instr_begin;
    ignore_non_whitespace(code, code_sz, &idx);
    //TODO
  }

  for (word i = 0; i < array_len(lbl_refs); ++i){
    lbl_info * l_info = (lbl_info*)lbl_refs;
    //arr[l_info->addr] |= TODO
  }

  array_delete(heap, lbl_refs);
  return arr;

 error:
  array_delete(heap, arr);
  array_delete(heap, lbl_refs);
  return 0;
}


void run(word bytecode, word bc_sz){
  
}
