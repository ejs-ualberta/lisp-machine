#include "config.h"

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
  word name[3];
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
  [ace]=(op_data){3, {'a', 'c', 'e'}},
  [ads]=(op_data){3, {'a', 'd', 's'}},
  [sbs]=(op_data){3, {'s', 'b', 's'}},
  [mls]=(op_data){3, {'m', 'l', 's'}},
  [mlu]=(op_data){3, {'m', 'l', 'u'}},
  [dvs]=(op_data){3, {'d', 'v', 's'}},
  [dvu]=(op_data){3, {'d', 'v', 'u'}},
  [and]=(op_data){3, {'a', 'n', 'd'}},
  [orr]=(op_data){3, {'o', 'r', 'r'}},
  [xor]=(op_data){3, {'x', 'o', 'r'}},
  [nor]=(op_data){3, {'n', 'o', 'r'}},
  [shf]=(op_data){3, {'s', 'h', 'f'}},
  [ldr]=(op_data){3, {'l', 'd', 'r'}},
  [str]=(op_data){3, {'s', 't', 'r'}},
  [jnc]=(op_data){3, {'j', 'n', 'c'}}, // NOTE: use sr for uncond jump.
  [exc]=(op_data){3, {'e', 'x', 'c'}}, // interrupt number, argc, argv *
};

typedef struct label_addr{
  word * label;
  word lbl_addr;
} lbl_info;

const word lbl_info_sz = sizeof(lbl_info)/sizeof(word);


word * compile(word * code, word code_sz){
  
}


void run(word bytecode, word bc_sz){
  
}
