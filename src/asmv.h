#ifndef smv_smvm_asmv_h
#define smv_smvm_asmv_h

#include "smvm.h"
#include "util.h"

typedef struct asmv {
  char *code;  // input
  listmv(asmv_inst) instructions;
  listmv(asmv_label) label_refs;
  listmv(label_reference) label_addrs;
  listmv(smvm_syscall) syscalls;
  smvm_header header;
  listmv(u8) memory;
  listmv(u8) bytecode;
  u64 index;
  bool panic_mode;
} asmv;

typedef enum asmv_error {
  asmv_all_ok = 0,
  asmv_incomplete_inst = 1,
  asmv_incomplete_str = 2,
  asmv_label_missing = 3,
  asmv_immediate_x = 4,
  asmv_misc_error = 5,
} asmv_error;

typedef struct asmv_op_data {
  enum asmv_op_type {
    asmv_reg_type,
    asmv_num_type,
    asmv_unum_type,
    asmv_fnum_type,
    asmv_str_type,
    asmv_chr_type,
    asmv_label_type
  } type;
  union {
    smvm_register reg;
    i64 num;
    u64 unum;
    f64 fnum;
    listmv str;
  };
} asmv_op_data;

typedef struct asmv_operand {
  asmv_op_data data;
  u8 offset;
  smvm_mode mode : 2;
  smvm_data_width width : 2;
  smvm_data_width size : 2;
} asmv_operand;

typedef struct asmv_inst {
  smvm_opcode code;
  union {
    asmv_operand operands[3];
    listmv str;
  };
  bool eof : 1;
  bool label : 1;
  u64 label_index;
  u64 index;
  asmv_error error;
} asmv_inst;

typedef struct asmv_label {
  listmv str;
  u64 address;
  u64 index;  // index in instruction array
} asmv_label;

typedef struct label_reference {
  u64 inst_index;
  u8 op_index;
} label_reference;

void asmv_init(asmv *as, smvm *vm);
void asmv_assemble(asmv *as);
asmv_inst asmv_lex_inst(asmv *as);
void asmv_free(asmv *as);

char asmv_current(asmv *a);
char asmv_peek(asmv *a);
char asmv_peek2(asmv *a);
char asmv_prev(asmv *a);
char asmv_prev2(asmv *a);
char asmv_next(asmv *a);
void asmv_skip(asmv *a);

// helper function
u8 asmv_parse_register(asmv *as);

#endif