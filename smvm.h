#ifndef smv_smvm_impl_h
#define smv_smvm_impl_h

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef double f64;
typedef float f32;

/*** util ***/

typedef struct listmv {
  void *data;
  u64 len;
  u64 cap;
  long size;
} listmv;

void listmv_init(listmv *ls, long size);
void listmv_push(listmv *ls, void *data);
void listmv_push_array(listmv *ls, void *data, size_t num);
void *listmv_at(listmv *ls, u64 index);
void listmv_free(listmv *ls);

// temp helper
void print_memory(void *arr, int num) {
  unsigned char *p = (unsigned char *)arr;
  for (int i = 0; i < num; i++) {
    unsigned char byte = p[i];
    for (int j = 7; j >= 0; j--) {
      printf("%d", (byte >> j) & 0x01);
      if (j % 8 == 0)
        printf("|");
      else if (j % 4 == 0)
        printf(" ");
    }
  }
  printf("\n");
}

/*** vm ***/

#define smvm_register_num (5)

typedef struct smvm {
  listmv memory;
  listmv bytecode;
  listmv stack;
  // 000 - ip: instruction pointer
  // 001 - a:  NOT accumulator, rather a
  // 010 - b:  just b
  // 011 - c:  counter register
  // 100 - d:  destination register
  u64 registers[smvm_register_num];
  u16 flags;
} smvm;

typedef enum smvm_opcode {
  op_halt = 0b000000,  // halt <3 bit error code>
  op_mov = 0b000001,   // mov x y
  op_swap = 0b000010,  // swap x y
  op_lea = 0b000011,   // lea x y
  op_add = 0b000100,   // add x y z
  op_addu = 0b000101,  // addu x y z
  op_addf = 0b000110,  // addf x y z
  op_sub = 0b000111,   // sub x y z
  op_subu = 0b001000,  // subu x y z
  op_subf = 0b001001,  // subf x y z
  op_mul = 0b001010,   // mul x y z
  op_mulu = 0b001011,  // mulu x y z
  op_mulf = 0b001100,  // mulf x y z
  op_div = 0b001101,   // div x y z
  op_divu = 0b001110,  // divu x y z
  op_divf = 0b001111,  // divf x y z
  op_inc = 0b010000,   // inc x
  op_dec = 0b010001,   // dec x
  op_and = 0b010010,   // and x y z
  op_or = 0b010011,    // or x y z
  op_xor = 0b010100,   // xor x y z
  op_shl = 0b010101,   // shl x y z
  op_shr = 0b010110,   // shr x y z
  op_shli = 0b010111,  // shli x y z
  op_shri = 0b011000,  // shri x y z
  op_slc = 0b011001,   // slc x y z
  op_src = 0b011010,   // src x y z
  op_jmp = 0b011011,   // jmp (to [d]) i.e. mov [d]
  op_je = 0b011100,    // if (a == b) jmp (to [d])
  op_jne = 0b011101,   // if (a != b) jmp (to [d])
  op_jl = 0b011110,    // if (a < b) jmp (to [d])
  op_loop = 0b011111,  // if (c > 0) dec c ; jmp (to [d])
  op_call = 0b100000,  // call [d]
  // proc x start <opcodes> return
  op_proc = 0b100001,
  op_start = 0b100010,
  op_return = 0b100011,
  op_push = 0b100100,  // push x
  op_pop = 0b100101,   // pop x
  // NOTE: 1111__ > print instructions
  // these might be temporary instructions
  op_printi = 0b111100 - 22,  // print integer
  op_printu = 0b111101 - 22,  // print unsigned integer
  op_printf = 0b111110 - 22,  // print float
  op_prints = 0b111111 - 22,  // print string
} smvm_opcode;

typedef enum smvm_register {
  reg_a = 0b000,
  reg_b = 0b001,
  reg_c = 0b010,
  reg_d = 0b011,
  reg_ip = 0b100,
  reg_none = 0b111,
} smvm_register;

typedef enum smvm_register_width {
  smvm_reg8 = 0b00,
  smvm_reg16 = 0b01,
  smvm_reg32 = 0b10,
  smvm_reg64 = 0b11,
} smvm_data_width;

typedef enum smvm_flag {
  flag_o = 1,       // overflow
  flag_d = 1 << 1,  // direction
  flag_t = 1 << 2,  // trap ~TODO add errors~
  flag_s = 1 << 3,  // sign
  flag_z = 1 << 4,  // zero
} smvm_flag;

typedef enum smvm_mode {
  // mode takes 2 bits
  mode_register = 0b00,    // r
  mode_immediate = 0b01,   // i
  mode_indirect = 0b10,    // a
  mode_direct = 0b11,      // d
  mode_implicit = 0b1111,  // todo remove
  // implicit type is quite literally, implicit; implicit instructions are
  // simply run without checking the "mode" first, unlike other instructions
} smvm_mode;

void smvm_init(smvm *vm);
void smvm_assemble(smvm *vm, char *code, bool cache);
void smvm_execute(smvm *vm);
void smvm_free(smvm *vm);

/* vm - helpers */
u64 smvm_fetch_u64(smvm *vm);  // fetches 8 byte in native endian
// fetches operands and performs operations
static void smvm_handle_op(smvm *vm, u8 byte0,
                           void (*op_func)(smvm *, u8, u8, u64),
                           smvm_mode mode);
static void smvm_handle_op_implicit(smvm *vm, smvm_opcode code);
// converts to native endian from little-endian, TODO
void *convert_endian(void *arr, int num);
static void update_flags(smvm *vm, u64 result);

/*** vm - inline helpers ***/

static inline u64 repr_f64(f64 data) { return *(u64 *)&data; }

static inline smvm_data_width min_space_neededu(u64 data) {
  if ((data >> 8) == 0) return smvm_reg8;
  if ((data >> 16) == 0) return smvm_reg16;
  if ((data >> 32) == 0) return smvm_reg32;
  return smvm_reg64;
}

static smvm_data_width min_space_needed(i64 data) {
  if ((data >> 7) == 0 || (data >> 7) == -1) return smvm_reg8;
  if ((data >> 15) == 0 || (data >> 15) == -1) return smvm_reg16;
  if ((data >> 31) == 0 || (data >> 31) == -1) return smvm_reg32;
  return smvm_reg64;
}

static inline void smvm_ip_inc(smvm *vm) { vm->registers[reg_ip]++; }

static inline u8 *smvm_fetch_inst_addr(smvm *vm) {
  return (u8 *)listmv_at(&vm->bytecode, vm->registers[reg_ip]);
}

static inline u16 smvm_get_flag(smvm *vm, smvm_flag flag) {
  return vm->flags & flag;
}

static inline void smvm_set_flag(smvm *vm, smvm_flag flag) {
  vm->flags |= flag;
}

static inline void smvm_reset_flag(smvm *vm, smvm_flag flag) {
  vm->flags &= ~flag;
}

/*** vm - implementation ***/

void smvm_init(smvm *vm) {
  listmv_init(&vm->bytecode, sizeof(u8));
  listmv_init(&vm->memory, sizeof(u64));
  listmv_init(&vm->stack, sizeof(u64));

  for (int i = 0; i < smvm_register_num; i++) vm->registers[i] = 0;
}

/* NOTE these aren't inline helpers! these are the real deal */

static inline void add_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regx] = (i64)vm->registers[regy] + data;
  update_flags(vm, vm->registers[regx]);
}

static inline void addu_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regx] = vm->registers[regy] + data;
  update_flags(vm, vm->registers[regx]);
}

static inline void addf_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  f64 *dest = (f64 *)&vm->registers[regx];
  f64 *src1 = (f64 *)&vm->registers[regy];
  f64 *src2 = (f64 *)&data;
  *dest = *src1 + *src2;
}

static inline void sub_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regx] = (i64)vm->registers[regy] - (i64)data;
  update_flags(vm, vm->registers[regx]);
}

static inline void subu_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regx] = vm->registers[regy] - data;
  update_flags(vm, vm->registers[regx]);
}

static inline void subf_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  f64 *dest = (f64 *)&vm->registers[regx];
  f64 *src1 = (f64 *)&vm->registers[regy];
  f64 *src2 = (f64 *)&data;
  *dest = *src1 - *src2;
}

static inline void mul_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regx] = (i64)vm->registers[regy] * (i64)data;
  update_flags(vm, vm->registers[regx]);
}

static void mulu_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regx] = vm->registers[regy] * data;
  update_flags(vm, vm->registers[regx]);
}

static void mulf_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  f64 *dest = (f64 *)&vm->registers[regx];
  f64 *src1 = (f64 *)&vm->registers[regy];
  f64 *src2 = (f64 *)&data;
  *dest = *src1 * *src2;
}

static void div_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regx] = (i64)vm->registers[regy] / (i64)data;
  update_flags(vm, vm->registers[regx]);
}

static void divu_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regx] = vm->registers[regy] / data;
  update_flags(vm, vm->registers[regx]);
}

static void divf_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  f64 *dest = (f64 *)&vm->registers[regx];
  f64 *src1 = (f64 *)&vm->registers[regy];
  f64 *src2 = (f64 *)&data;
  *dest = *src1 / *src2;
}

static inline void mov_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  vm->registers[regy] = data;
}

static inline void print_int_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  printf("%d\n", data);
}

static inline void print_intu_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  printf("%u\n", data);
}

static inline void print_float_op(smvm *vm, u8 regx, u8 regy, u64 data) {
  printf("%lf\n", *(f64 *)&data);
}

// realer deal (maybe not)
// TODO error return type
// TODO refactor... yeah, again!
void smvm_execute(smvm *vm) {
  while (true) {
    u8 byte0 = *smvm_fetch_inst_addr(vm);
    smvm_ip_inc(vm);
    u8 opcode = byte0 >> 2;
    smvm_mode mode = (smvm_mode)(byte0 & 0b11);

    switch (opcode) {
      case op_halt:
        return;

      case op_mov:
        smvm_handle_op(vm, byte0, mov_op, mode);
        break;

      case op_add:
        smvm_handle_op(vm, byte0, add_op, mode);
        break;
      case op_addu:
        smvm_handle_op(vm, byte0, addu_op, mode);
        break;
      case op_addf:
        smvm_handle_op(vm, byte0, addf_op, mode);
        break;

      case op_sub:
        smvm_handle_op(vm, byte0, sub_op, mode);
        break;
      case op_subu:
        smvm_handle_op(vm, byte0, subu_op, mode);
        break;
      case op_subf:
        smvm_handle_op(vm, byte0, subf_op, mode);
        break;

      case op_mul:
        smvm_handle_op(vm, byte0, mul_op, mode);
        break;
      case op_mulu:
        smvm_handle_op(vm, byte0, mulu_op, mode);
        break;
      case op_mulf:
        smvm_handle_op(vm, byte0, mulf_op, mode);
        break;

      case op_div:
        smvm_handle_op(vm, byte0, div_op, mode);
        break;
      case op_divu:
        smvm_handle_op(vm, byte0, divu_op, mode);
        break;
      case op_divf:
        smvm_handle_op(vm, byte0, divf_op, mode);
        break;

      case op_printi:
        smvm_handle_op(vm, byte0, print_int_op, mode);
        break;
      case op_printu:
        smvm_handle_op(vm, byte0, print_intu_op, mode);
        break;
      case op_printf:
        smvm_handle_op(vm, byte0, print_float_op, mode);
        break;
        // TODO add back print str later

      default:  // error
        break;
    }
  }
}

void smvm_free(smvm *vm) {
  listmv_free(&vm->memory);
  listmv_free(&vm->bytecode);
  listmv_free(&vm->stack);
}

/*** vm - assembler ***/

// these are only lexical errors i.e. errors like zero division error,
// labelling error aren't handled
typedef enum asmv_error {
  asmv_all_ok = 0,
  asmv_incomplete_inst = 1,
  asmv_incomplete_str = 2,
  asmv_immediate_x = 3,
  asmv_misc_error = 4,
} asmv_error;

typedef struct asmv_operand {
  smvm_mode mode;
  enum asmv_operand_type {
    asmv_reg,
    asmv_num,
    asmv_unum,
    asmv_fnum,
    asmv_str,
    asmv_chr,
  } type;
  u8 width;
  union {
    smvm_register reg;
    i64 num;
    u64 unum;
    f64 fnum;
    listmv str;
  };
} asmv_operand;

typedef struct asmv_inst {
  smvm_opcode code;
  union {
    asmv_operand operands[3];
    listmv str;
  };
  bool eof : 1;
  bool label : 1;
  asmv_error error;
} asmv_inst;

// this is for the assembler
// todo, implement hashing or something
struct temp_inst_table {
  char *name;
  u8 str_size : 4;
  u8 num_ops : 4;
} instruction_table[] = {
    [op_halt] = {"halt", 4, 0},     [op_mov] = {"mov", 3, 2},
    [op_swap] = {"swap", 4, 2},     [op_lea] = {"lea", 3, 2},
    [op_addu] = {"addu", 4, 3},     [op_addf] = {"addf", 4, 3},
    [op_add] = {"add", 3, 3},       [op_subu] = {"subu", 4, 3},
    [op_subf] = {"subf", 4, 3},     [op_sub] = {"sub", 3, 3},
    [op_mulu] = {"mulu", 4, 3},     [op_mulf] = {"mulf", 4, 3},
    [op_mul] = {"mul", 3, 3},       [op_divu] = {"divu", 4, 3},
    [op_divf] = {"divf", 4, 3},     [op_div] = {"div", 3, 3},
    [op_inc] = {"inc", 3, 1},       [op_dec] = {"dec", 3, 1},
    [op_and] = {"and", 3, 3},       [op_or] = {"or", 2, 3},
    [op_xor] = {"xor", 3, 3},       [op_shli] = {"shli", 4, 3},
    [op_shl] = {"shl", 3, 3},       [op_shri] = {"shri", 4, 3},
    [op_shr] = {"shr", 3, 3},       [op_slc] = {"slc", 3, 3},
    [op_src] = {"src", 3, 3},       [op_jmp] = {"jmp", 3, 0},
    [op_je] = {"je", 2, 3},         [op_jne] = {"jne", 3, 3},
    [op_jl] = {"jl", 2, 3},         [op_loop] = {"loop", 4, 2},
    [op_call] = {"call", 4, 1},     [op_proc] = {"proc", 4, 1},
    [op_start] = {"start", 5, 0},   [op_return] = {"return", 6, 0},
    [op_push] = {"push", 4, 1},     [op_pop] = {"pop", 3, 1},
    [op_printi] = {"printi", 6, 1}, [op_printu] = {"printu", 6, 1},
    [op_printf] = {"printf", 6, 1}, [op_prints] = {"prints", 6, 1}};

#define instruction_table_len \
  (sizeof(instruction_table) / sizeof(*instruction_table))

static u8 parse_register(char **code) {
  char *backup = *code;

  if (**code == 'i' && (*code)[1] == 'p' && !isalnum((*code)[2])) {
    *code += 2;
    return (smvm_reg64 << 3) | reg_ip;
  };

  if (**code == 'a' || **code == 'b' || **code == 'c' || **code == 'd') {
    (*code)++;
    if (**code == '8') {
      (*code)++;
      return (smvm_reg8 << 3) | (reg_a + (*code)[-1] - 'a');
    }
    if (**code == '1' && (*code)[1] == '6') {
      *code += 2;
      return (smvm_reg16 << 3) | (reg_a + (*code)[-2] - 'a');
    }
    if (**code == '3' && (*code)[1] == '2') {
      *code += 2;
      return (smvm_reg32 << 3) | (reg_a + (*code)[-2] - 'a');
    }
    if (**code == '6' && (*code)[1] == '4') {
      *code += 2;
      return (smvm_reg64 << 3) | (reg_a + (*code)[-2] - 'a');
    }

    if (!isdigit(**code))
      return (smvm_reg64 << 3) | (reg_a + (*code)[-1] - 'a');
  }

  *code = backup;
  return reg_none;
}

// TODO, floating point numbers
static u64 parse_number(char **code) {
  u64 num = 0;
  bool neg = false;
  if (**code == '-') {
    neg = true;
    (*code)++;
  }

  while (isdigit(**code)) {
    num = num * 10 + (**code - '0');
    (*code)++;
    if (**code == '_') (*code)++;  // something like 1_00_000 is valid
  }
  return neg ? -num : num;
}

// I hate nesting
static asmv_inst smvm_lex_inst(char **code) {
  char buffer[512];
  u64 index = 0;
  asmv_operand op;
  asmv_inst inst = {.error = asmv_all_ok, .eof = false, .label = false};

  while (isspace(**code)) (*code)++;

  if (**code == '\0') return ((asmv_inst){.eof = true});

  // TODO label
  if (**code == '.') return (asmv_inst){.label = true};

  if (!isalpha(**code)) return (asmv_inst){.error = asmv_misc_error};

  while (isalpha((*code)[index])) index++;
  strncpy(buffer, *code, index);
  buffer[index + 1] = '\0';
  *code += index;

  for (int i = 0; i < instruction_table_len; i++) {
    if (strncmp(instruction_table[i].name, buffer,
                instruction_table[i].str_size))
      continue;

    inst.code = i;

    for (int j = 0; j < instruction_table[i].num_ops; j++) {
      while (isspace(**code)) (*code)++;

      if (**code == '-' || isdigit(**code)) {
        // immediate mode, handle numbers
        // TODO: handle bases and .d+ (clashes with labels right now)
        // sub 12 b c makes no sense, 12 = b + c what??
        if (j == 0) return (asmv_inst){.error = asmv_immediate_x};
        // handle immediate addressing
        // either u or f, otherwise irrelevant
        char discriminator =
            instruction_table[i].name[instruction_table[i].str_size - 1];

        op.mode = mode_immediate;
        u64 num = parse_number(code);
        if (discriminator == 'u') {
          op.type = asmv_unum;
          op.num = num;
          op.width = min_space_neededu(op.num);
        } else if (discriminator == 'f') {
          // TODO, floating point numbers
          op.type = asmv_fnum;
          // op.num = 0;  // TODO;
          op.width = min_space_needed(op.num);
        } else {
          op.type = asmv_num;
          op.unum = *(i64 *)&num;
          op.width = min_space_needed(op.num);
        }
      } else if (**code == '@') {
        (*code)++;
        while (isspace(**code)) (*code)++;
        if (isdigit(**code)) {
          // if number, direct addressing mode
          // we don't check for - here since addresses are non-negative
          op.mode = mode_direct;
          op.unum = parse_number(code);
          op.width = min_space_neededu(op.unum);
        } else if (**code == 'r') {
          // else if register, indirect addressing mode
          (*code)++;
          smvm_register reg = parse_register(code);
          // TODO handle reg_none better
          if (reg == reg_none) break;
          op.mode = mode_indirect;
          op.width = reg >> 3;
          op.reg = reg & 0b111;

        } else {
          // TODO, handle error
        }
      } else if (**code == 'r') {
        (*code)++;
        smvm_register reg = parse_register(code);  // gives width|reg
        // TODO handle reg_none better mayhaps
        // time will tell
        if (reg == reg_none) continue;
        op.mode = mode_register;
        op.width = reg >> 3;
        op.reg = reg & 0b111;
      } else if (**code == '"' && inst.code == op_prints) {
        // this is immediate mode too actually
        (*code)++;
        index = 0;
        while (*(*code + index) != '"') i++;
        //  TODO matching strings
      } else if (**code == '.' && isalpha((*code)[1])) {
        // match for labels
        **code++;
        while (isalnum(**code)) (*code)++;
        // TODO, save label
        // op.mode = mode_direct;
        // listmv_init(&op.str, sizeof(char));
      } else {
        // TODO edge cases, error handling
      }
      inst.operands[j] = op;
    }
    break;
  }
  return inst;
}

void smvm_assemble(smvm *vm, char *code, bool cache) {
  while (*code != '\0') {
    asmv_inst inst = smvm_lex_inst(&code);
    if (inst.eof) break;
    if (inst.error != asmv_all_ok) {
      // TODO, better error handling?
      printf("error in assembling: %d\n", inst.error);
      continue;
    }

    const u8 num_ops = instruction_table[inst.code].num_ops;

    if (num_ops == 0) {
      listmv_push(&vm->bytecode, &inst.code);
      continue;
    }

    u8 primary_bytes[4] = {inst.code, 0, 0, 0};
    u8 immediate_bytes[24] = {0};  // data/address bytes
    u64 immediate_size = 0;

    for (int i = 0; i < instruction_table[inst.code].num_ops; i++) {
      asmv_operand op = inst.operands[i];
      // set the info and mode bits
      primary_bytes[i] |= op.mode << 6;
      primary_bytes[1] |= op.width << (i * 2);

      // set the register/data bits
      u8 byte = i == 2 ? 3 : 2;
      u8 offset = i == 1 ? 3 : 0;
      if (op.mode == mode_register || op.mode == mode_indirect) {
        primary_bytes[byte] |= op.reg << offset;
      } else if (op.mode == mode_indirect || op.mode == mode_immediate) {
        u8 data_size = 1 << op.width;  // ranges from 1 to 8
        immediate_size += data_size;
        for (int n = 0; n < data_size; n++) {
          immediate_bytes[immediate_size - n - 1] = (op.num >> (n * 8)) & 0xff;
        }
      }
    }
    printf("---\n");
    print_memory(primary_bytes, num_ops == 3 ? 4 : 3);
    printf("---\n");
    print_memory(immediate_bytes, immediate_size);
    printf("---\n");

    listmv_push_array(&vm->bytecode, primary_bytes, num_ops == 3 ? 4 : 3);
    listmv_push_array(&vm->bytecode, immediate_bytes, immediate_size);
  }
}

/* vm - helpers - implementation */

u64 smvm_fetch_u64(smvm *vm) {
  u64 val;
  memcpy(&val, smvm_fetch_inst_addr(vm), sizeof(u64));
  smvm_ip_inc(vm);
  return val;
}

// assuming little-endian, no-op
void *convert_endian(void *arr, int num) { return arr; }

// TODO, data
static void smvm_handle_op(smvm *vm, u8 byte0,
                           void (*op_func)(smvm *, u8, u8, u64),
                           smvm_mode mode) {
  u8 byte1 = *smvm_fetch_inst_addr(vm);
  smvm_ip_inc(vm);
  u8 regx, regy, regz;
  u64 addr_data;

  switch (mode) {
    case mode_register: {
      regx = (byte0 >> 5 & 0b100) | (byte1 >> 6 & 0b011);
      regy = (byte1 >> 3) & 0b111;
      regz = byte1 & 0b111;
      op_func(vm, regx, regy, vm->registers[regz]);
      break;
    }

    case mode_indirect: {
      regx = (byte0 >> 5 & 0b100) | (byte1 >> 6 & 0b011);
      regy = (byte1 >> 3) & 0b111;
      regz = byte1 & 0b111;
      addr_data = *((u64 *)listmv_at(&vm->memory, vm->registers[regz]));
      op_func(vm, regx, regy, addr_data);
      break;
    }

    case mode_immediate: {
      regx = (byte1 >> 3) & 0b111;
      regy = byte1 & 0b111;
      addr_data = *((u64 *)listmv_at(&vm->bytecode, vm->registers[reg_ip]));
      vm->registers[reg_ip] += 8;  // move pointer forward after fetching
      op_func(vm, regx, regy, addr_data);
      break;
    }

    case mode_direct: {
      regx = (byte0 >> 5 & 0b100) | (byte1 >> 6 & 0b011);
      regy = (byte1 >> 3) & 0b111;
      addr_data = *((u64 *)listmv_at(&vm->bytecode, vm->registers[reg_ip]));
      vm->registers[reg_ip] += 8;  // move pointer forward after fetching
      op_func(vm, regx, regy,
              (u64 *)vm->memory.data - vm->registers + addr_data);
      break;
    }

    case mode_implicit: {
      op_func(vm, 0, 0, 0);  // placeholder
      break;
    }

    default:  // TODO error handling
      break;
  }
}

static void update_flags(smvm *vm, u64 result) {
  if (result == 0)
    smvm_set_flag(vm, flag_z);
  else
    smvm_reset_flag(vm, flag_z);

  if (result >> 63)
    smvm_set_flag(vm, flag_s);
  else
    smvm_reset_flag(vm, flag_s);
}

/*** util - implementation ***/

void listmv_init(listmv *ls, long size) {
  ls->len = 0;
  ls->cap = 1;
  ls->size = size;
  ls->data = calloc(ls->cap, size);
}

void listmv_push(listmv *ls, void *data) {
  if (ls->len >= ls->cap) {
    ls->cap *= 2;
    ls->data = realloc(ls->data, ls->cap * ls->size);
  }
  memcpy((char *)ls->data + ls->len * ls->size, data, ls->size);
  ls->len++;
}

void listmv_push_array(listmv *ls, void *data, size_t num) {
  while (ls->len + num >= ls->cap) {
    ls->cap *= 2;
    ls->data = realloc(ls->data, ls->cap * ls->size);
  }
  memcpy((char *)ls->data + ls->len * ls->size, data, num * ls->size);
  ls->len += num;
}

void *listmv_at(listmv *ls, u64 index) {
  return (char *)ls->data + index * ls->size;
}

void listmv_free(listmv *ls) { free(ls->data); }

#endif
