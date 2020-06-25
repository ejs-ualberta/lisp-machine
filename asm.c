#include "config.h"

const word opcode_name_len = 4;
const word bits = sizeof(word) * 8;
const word reg_des = 'r';
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
  word lbl_addr;
} lbl_info;

const word lbl_info_sz = sizeof(lbl_info)/sizeof(word);


word whitespace(word ch){
  return ch == ' ' || ch == '\n' || ch == '\t';
}


void ignore_whitespace(word * code, word code_sz, word * idx){
  word i;
  for (i = *idx; i < code_sz && whitespace(code[i]); ++i){}
  *idx = i;
}


word match_opcode(word * code, word code_sz, word * index){
  for (word i = 0; i < n_instrs; ++i){
    word j = 0;
    word ch;
    for (; (ch = instructions[i].name[j]); ++j){
      word idx = *index + j;
      if (idx > code_sz){return (word)-1;}
      if (ch != code[idx]){goto fail;}
    }
    *index += j;
    return i;
  fail:;
  }
  return (word)-1;
}


// code is just a regular word *, not a special datastructure. 
word * compile(word * heap, word * code, word code_sz){
  // Use magic number to estimate of the amount of characters per instr;
  // opc arg arg arg imm\n
  word * arr = array(heap, code_sz / 32, 1);
  if (!arr){return (word*)0;}

  word prgm_ctr = 0;
  for (word idx = 0; idx < code_sz; ++idx){
    ignore_whitespace(code, code_sz, &idx);
    word opcode = match_opcode(code, code_sz, &idx);
    if (opcode >= n_instrs){return (word*)0;}

    word args[mx_reg_args];
    word n_args = instructions[opcode].n_args;
    for (word i = 0; i < n_args; ++i){
      
    } 
  }

  /* //TODO: Make sure interpreter asm code sets the type. */
  /* word * asm_fn = object(heap, (word*)0, array_size(arr), arr, array_size(arr)); */
  /* array_delete(heap, arr); */
  /* return asm_fn; */
  return arr;
}


void run(word bytecode, word bc_sz){
  
}
