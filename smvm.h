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
void *listmv_pop(listmv *ls);
void *listmv_pop_array(listmv *ls, u64 num);
void listmv_grow(listmv *ls, u64 new_cap);
void *listmv_at(listmv *ls, u64 index);
void listmv_free(listmv *ls);
// just a helper
void print_memory(void *arr, int num);

/* util - inline functions */

static inline void mov_mem(u8 *dest, u8 *src, u64 size) {
  while (size--) *dest++ = *src++;
}

static inline void mov_mem_reverse(u8 *dest, u8 *src, u64 size) {
  dest += size - 1;
  while (size--) *dest-- = *src++;
}

/*** vm ***/

/* vm - delcarations */

#define smvm_register_num (8)

typedef struct smvm {
  listmv memory;
  listmv bytecode;
  listmv stack;
  u64 registers[smvm_register_num];
  u8 flags;
  bool little_endian;
  // this is all cache, maybe I should make another struct
  i64 *pointers[3];
  i64 data[3];
  u8 widths[3];
  u8 offset;
} smvm;

typedef enum smvm_opcode {
  op_halt = 0b000000,
  op_mov  = 0b000001,
  op_swap = 0b000010,
  op_lea  = 0b000011,
  op_add  = 0b000100,
  op_addu = 0b000101,
  op_addf = 0b000110,
  op_sub  = 0b000111,
  op_subu = 0b001000,
  op_subf = 0b001001,
  op_mul  = 0b001010,
  op_mulu = 0b001011,
  op_mulf = 0b001100,
  op_div  = 0b001101,
  op_divu = 0b001110,
  op_divf = 0b001111,
  op_inc  = 0b010000,
  op_dec  = 0b010001,
  op_and  = 0b010010,
  op_or   = 0b010011,
  op_xor  = 0b010100,
  op_shl  = 0b010101,
  op_shr  = 0b010110,
  op_shli = 0b010111,
  op_shri = 0b011000,
  op_slc  = 0b011001,
  op_src  = 0b011010,
  op_jmp  = 0b011011,
  op_je   = 0b011100,
  op_jne  = 0b011101,
  op_jl   = 0b011110,
  op_loop = 0b011111,
  op_call = 0b100000,
  op_ret  = 0b100001,
  op_push = 0b100010,
  op_pop  = 0b100011,
  op_sys  = 0b100100,
  op_getu = 0b100101,
  op_puti = 0b100110,
  op_putu = 0b100111,
  op_putf = 0b101000,
  op_puts = 0b101001
} smvm_opcode;

typedef enum smvm_register {
  reg_a    = 0b000,
  reg_b    = 0b001,
  reg_c    = 0b010,
  reg_d    = 0b011,
  reg_e    = 0b100,
  reg_sp   = 0b101,
  reg_ip   = 0b110,
  reg_none = 0b1000
} smvm_register;

typedef enum smvm_data_width {
  smvm_reg8  = 0b00,
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
  mode_register  = 0b00,    // r
  mode_indirect  = 0b01,    // x
  mode_immediate = 0b10,    // i
  mode_direct    = 0b11,    // d
  mode_implicit  = 0b1111,  // TODO remove?
  // implicit type is quite literally, implicit; implicit instructions are
  // simply run without checking the "mode" first, unlike other instructions
} smvm_mode;

void smvm_init(smvm *vm);
void smvm_assemble(smvm *vm, char *code, bool cache);
void smvm_execute(smvm *vm);
void smvm_free(smvm *vm);

/* vm - helpers */

// converts to native endian from little-endian, TODO
static void stack_push_u64(smvm *vm, u64 value);
static u64 stack_pop_u64(smvm *vm);
static bool is_little_endian();
/* vm - inline helpers */

static inline void update_stack_pointer(smvm *vm) {
  vm->registers[reg_sp] = vm->stack.len;
}

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

static inline void smvm_ip_inc(smvm *vm, u64 inc) {
  vm->registers[reg_ip] += inc;
}

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

// this feels so redundant, if only I wasn't lazy and wrote a macro or
// something...
void trap_fn(smvm *vm);
void mov_fn(smvm *vm);
void swap_fn(smvm *vm);
void lea_fn(smvm *vm);
void add_fn(smvm *vm);
void addu_fn(smvm *vm);
void addf_fn(smvm *vm);
void sub_fn(smvm *vm);
void subu_fn(smvm *vm);
void subf_fn(smvm *vm);
void mul_fn(smvm *vm);
void mulu_fn(smvm *vm);
void mulf_fn(smvm *vm);
void div_fn(smvm *vm);
void divu_fn(smvm *vm);
void divf_fn(smvm *vm);
void inc_fn(smvm *vm);
void dec_fn(smvm *vm);
void and_fn(smvm *vm);
void or_fn(smvm *vm);
void xor_fn(smvm *vm);
void shl_fn(smvm *vm);
void shr_fn(smvm *vm);
void shli_fn(smvm *vm);
void shri_fn(smvm *vm);
void slc_fn(smvm *vm);
void src_fn(smvm *vm);
void jmp_fn(smvm *vm);
void je_fn(smvm *vm);
void jne_fn(smvm *vm);
void jl_fn(smvm *vm);
void loop_fn(smvm *vm);
void call_fn(smvm *vm);
void ret_fn(smvm *vm);
void push_fn(smvm *vm);
void pop_fn(smvm *vm);
void sys_fn(smvm *vm);
void getu_fn(smvm *vm);
void puti_fn(smvm *vm);
void putu_fn(smvm *vm);
void putf_fn(smvm *vm);
void puts_fn(smvm *vm);

// TODO, implement hashing or something
typedef struct instruction_info {
  char *name;
  u8 str_size : 4;
  u8 num_ops : 4;
  void (*fn)(smvm *);
} instruction_info;

instruction_info instruction_table[] = {
    [op_halt] = {"halt", 4, 0, trap_fn}, [op_mov] = {"mov", 3, 2, mov_fn},
    [op_swap] = {"swap", 4, 2, swap_fn}, [op_lea] = {"lea", 3, 2, lea_fn},
    [op_addu] = {"addu", 4, 3, addu_fn}, [op_addf] = {"addf", 4, 3, addf_fn},
    [op_add] = {"add", 3, 3, add_fn},    [op_subu] = {"subu", 4, 3, subu_fn},
    [op_subf] = {"subf", 4, 3, subf_fn}, [op_sub] = {"sub", 3, 3, sub_fn},
    [op_mulu] = {"mulu", 4, 3, mulu_fn}, [op_mulf] = {"mulf", 4, 3, mulf_fn},
    [op_mul] = {"mul", 3, 3, mul_fn},    [op_divu] = {"divu", 4, 3, divu_fn},
    [op_divf] = {"divf", 4, 3, divf_fn}, [op_div] = {"div", 3, 3, div_fn},
    [op_inc] = {"inc", 3, 1, inc_fn},    [op_dec] = {"dec", 3, 1, dec_fn},
    [op_and] = {"and", 3, 3, and_fn},    [op_or] = {"or", 2, 3, or_fn},
    [op_xor] = {"xor", 3, 3, xor_fn},    [op_shli] = {"shli", 4, 3, shli_fn},
    [op_shl] = {"shl", 3, 3, shl_fn},    [op_shri] = {"shri", 4, 3, shri_fn},
    [op_shr] = {"shr", 3, 3, shr_fn},    [op_slc] = {"slc", 3, 3, slc_fn},
    [op_src] = {"src", 3, 3, src_fn},    [op_jmp] = {"jmp", 3, 1, jmp_fn},
    [op_je] = {"je", 2, 3, je_fn},       [op_jne] = {"jne", 3, 3, jne_fn},
    [op_jl] = {"jl", 2, 3, jl_fn},       [op_loop] = {"loop", 4, 2, loop_fn},
    [op_call] = {"call", 4, 1, call_fn}, [op_ret] = {"ret", 3, 0, ret_fn},
    [op_push] = {"push", 4, 1, push_fn}, [op_pop] = {"pop", 3, 1, pop_fn},
    [op_sys] = {"sys", 3, 1, sys_fn},    [op_getu] = {"getu", 4, 1, getu_fn},
    [op_puti] = {"puti", 4, 1, puti_fn}, [op_putu] = {"putu", 4, 1, putu_fn},
    [op_putf] = {"putf", 4, 1, putf_fn}, [op_puts] = {"puts", 4, 1, puts_fn}};

#define instruction_table_len \
  (sizeof(instruction_table) / sizeof(*instruction_table))

/*** vm - assembler ***/

// TODO, proper assembler - use struct asmv
typedef struct asmv {
  char *code;
  listmv instructions;
  listmv label_refs;
  listmv label_addrs;
  u64 index;
} asmv;

// these are only lexical errors i.e. errors like zero division error,
// labelling error aren't handled
typedef enum asmv_error {
  asmv_all_ok          = 0,
  asmv_incomplete_inst = 1,
  asmv_incomplete_str  = 2,
  asmv_label_missing   = 3,
  asmv_immediate_x     = 4,
  asmv_misc_error      = 5,
} asmv_error;

typedef struct asmv_operand {
  smvm_mode mode;
  enum asmv_operand_type {
    asmv_reg_type,
    asmv_num_type,
    asmv_unum_type,
    asmv_fnum_type,
    asmv_str_type,
    asmv_chr_type,
    asmv_label_type
  } type;
  smvm_data_width width : 4;
  smvm_data_width size : 4;
  union {
    smvm_register reg;
    i64 num;
    u64 unum;
    f64 fnum;
    listmv str;
  } data;
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

typedef struct asmv_label {
  listmv str;
  u64 address;
} asmv_label;

typedef struct label_reference {
  u64 inst_index;
  u8 op_index;
} label_reference;

// this isn't even... remotely useful
// but for the sake of some semblence of order, it exists now
#ifndef smv_smvm_header_only

/*** vm - implementation ***/

void smvm_init(smvm *vm) {
  *vm = (smvm){0};
  listmv_init(&vm->bytecode, sizeof(u8));
  listmv_init(&vm->memory, sizeof(u8));
  listmv_init(&vm->stack, sizeof(u8));
  vm->little_endian = is_little_endian();
}

// TODO error return type
void smvm_execute(smvm *vm) {
  while (true) {
    u8 *inst   = smvm_fetch_inst_addr(vm);
    u8 code    = inst[0] & 0x3f;
    u8 num_ops = instruction_table[code].num_ops;

    if (instruction_table[code].num_ops == 0) {
      instruction_table[code].fn(vm);
    } else {
      u8 reg[3] = {inst[2] & 7, (inst[2] >> 3) & 7, inst[3] & 7};
      u8 offset = num_ops != 3 ? 3 : 4;

      for (int i = 0; i < num_ops; i++)
        vm->widths[i] = 1 << ((inst[1] >> (i * 2)) & 3);

      for (int i = 0; i < num_ops; i++) {
        switch (inst[i] >> 6) {
          case mode_register: vm->pointers[i] = &vm->registers[reg[i]]; break;
          case mode_indirect:
            listmv_grow(&vm->memory, vm->registers[reg[i]] + vm->widths[i] + 1);
            vm->pointers[i] = listmv_at(&vm->memory, vm->registers[reg[i]]);
            break;
          case mode_direct: {
            u8 size     = 1 << (reg[i] & 3);
            u64 address = 0;
            // reg also holds size
            mov_mem((u8 *)&address, inst + offset, size);
            listmv_grow(&vm->memory, address + vm->widths[i] + 1);
            offset += size;
            vm->pointers[i] = listmv_at(&vm->memory, address);
            break;
          }
          case mode_immediate:
            u8 size     = 1 << (reg[i] & 3);
            vm->data[i] = 0;
            if (vm->little_endian)
              mov_mem_reverse((u8 *)(&vm->data[i]), inst + offset, size);
            else
              mov_mem((u8 *)(&vm->data[i]), inst + offset, size);
            offset += size;
            vm->pointers[i] = &vm->data[i];
            break;
          default:  // idk what this is for
            break;
        }
      }
      vm->offset = offset;
      instruction_table[code].fn(vm);
    }

    if (smvm_get_flag(vm, flag_t)) break;  // TODO, so much
  }
}

void smvm_free(smvm *vm) {
  listmv_free(&vm->memory);
  listmv_free(&vm->bytecode);
  listmv_free(&vm->stack);
}

/*** vm - assembler - implementation ***/
/* inline functions */
static inline u64 asmv_current(asmv *a) { return a->index; }
static inline u64 asmv_peek(asmv *a) { return a->index + 1; }
static inline u64 asmv_next(asmv *a) { return ++a->index; }

static u8 asmv_parse_register(char **code) {
  char *backup = *code;

  if (**code == 'i' && (*code)[1] == 'p' && !isalnum((*code)[2])) {
    *code += 2;
    return (smvm_reg64 << 3) | reg_ip;
  }

  if (**code == 's' && (*code)[1] == 'p' && !isalnum((*code)[2])) {
    *code += 2;
    return (smvm_reg64 << 3) | reg_sp;
  }

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
  u64 num  = 0;
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
  u64 index       = 0;
  asmv_operand op = {0};
  asmv_inst inst  = {.error = asmv_all_ok, .eof = false, .label = false};

  while (isspace(**code)) (*code)++;
  // ignore comments
  if (**code == ';' || **code == '#')
    while (**code != '\n') (*code)++;
  if (**code == '\0') return ((asmv_inst){.eof = true});

  // label
  if (**code == '.') {
    (*code)++;
    index = 0;
    while (isalnum((*code)[index])) index++;
    char eof = '\0';
    listmv_init(&inst.str, sizeof(char));
    listmv_push_array(&inst.str, *code, index);
    listmv_push(&inst.str, &eof);
    *code += index;
    inst.label = true;
    return inst;
  }

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
        if (instruction_table[i].num_ops > 1 && j == 0)
          return (asmv_inst){.error = asmv_immediate_x};
        // handle immediate addressing
        // either u or f, otherwise irrelevant
        char discriminator =
            instruction_table[i].name[instruction_table[i].str_size - 1];

        op.mode = mode_immediate;
        u64 num = parse_number(code);
        if (discriminator == 'u') {
          op.type     = asmv_unum_type;
          op.data.num = num;
          op.size     = min_space_neededu(op.data.num);
        } else if (discriminator == 'f') {
          // TODO, floating point numbers
          op.type = asmv_fnum_type;
          // op.data.num = 0;  // TODO;
          op.size = min_space_needed(op.data.num);
        } else {
          op.type      = asmv_num_type;
          op.data.unum = *(i64 *)&num;
          op.size      = min_space_needed(op.data.num);
        }
      } else if (**code == '@') {
        (*code)++;
        while (isspace(**code)) (*code)++;
        if (isdigit(**code)) {
          // if number, direct addressing mode
          // we don't check for - here since addresses are non-negative
          op.mode      = mode_direct;
          op.type      = asmv_reg_type;
          op.data.unum = parse_number(code);
          op.size      = min_space_neededu(op.data.unum);
          if (**code == '>') {
            (*code)++;
            if (**code == '6' && (*code)[1] == '4') {
              op.width = smvm_reg64;
              *code += 2;
            } else if (**code == '3' && (*code)[1] == '2') {
              op.width = smvm_reg32;
              *code += 2;
            } else if (**code == '1' && (*code)[1] == '6') {
              op.width = smvm_reg16;
              *code += 2;
            } else if (**code == '8') {
              op.width = smvm_reg8;
              (*code)++;
            }
          } else
            op.width = smvm_reg64;

        } else if (**code == 'r') {
          // else if register, indirect addressing mode
          (*code)++;
          smvm_register reg = asmv_parse_register(code);
          // TODO handle reg_none better
          if (reg == reg_none) break;
          op.mode     = mode_indirect;
          op.width    = reg >> 3;
          op.data.reg = reg & 0b111;
        } else {
          // TODO, handle error
        }
      } else if (**code == 'r') {
        (*code)++;
        smvm_register reg = asmv_parse_register(code);  // gives width|reg
        // TODO handle reg_none better mayhaps
        // time will tell
        if (reg == reg_none) continue;
        op.mode     = mode_register;
        op.width    = reg >> 3;
        op.data.reg = reg & 0b111;
      } else if (**code == '"' && inst.code == op_puts) {
        (*code)++;
        listmv_init(&op.data.str, sizeof(char));

        while (**code != '"') {
          char c = **code;
          // checks for `\n`, etc.
          if (c == '\\' && *(*code + 1) != '\0') {
            c = *++(*code);
            switch (c) {
              case 'n': c = '\n'; break;
              case 't': c = '\t'; break;
              case 'r': c = '\r'; break;
              case '\\':
              case '"': break;
              default: (*code)--; c = '\\';
            }
          }
          listmv_push(&op.data.str, &c);
          (*code)++;
        }

        listmv_push(&op.data.str, &(char){'\0'});

        op.type = asmv_str_type;
        op.mode = mode_register;
        (*code)++;
      } else if (**code == '.') {  // handling labels
        (*code)++;
        index = 0;
        while (isalnum((*code)[index])) index++;
        op.type  = asmv_label_type;
        op.mode  = mode_immediate;
        char eof = '\0';
        listmv_init(&op.data.str, sizeof(char));
        listmv_push_array(&op.data.str, *code, index);
        listmv_push(&op.data.str, &eof);
        *code += index;
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
  asmv assembler;
  u64 offset = 0;  // track offset

  assembler.code = code;
  listmv_init(&assembler.instructions, sizeof(asmv_inst));
  listmv_init(&assembler.label_addrs, sizeof(asmv_label));
  listmv_init(&assembler.label_refs, sizeof(label_reference));

  // first pass
  while (*code != '\0') {
    asmv_inst inst = smvm_lex_inst(&code);
    if (inst.eof) break;
    if (!inst.label) {
      if (instruction_table[inst.code].num_ops == 0)
        offset++;
      else {
        offset += instruction_table[inst.code].num_ops == 3 ? 4 : 3;
        for (int i = 0; i < instruction_table[inst.code].num_ops; i++) {
          // TODO efficiency for storing labels
          if (inst.operands[i].type == asmv_label_type)
            offset += 8;
          else if (inst.operands[i].type == asmv_str_type)
            offset += inst.operands[i].data.str.len;
          else if (inst.operands[i].mode > 1)
            offset += 1 << inst.operands[i].size;
        }
      }
    } else {
      asmv_label label = {.address = offset, .str = inst.str};
      listmv_push(&assembler.label_addrs, &label);
    }

    listmv_push(&assembler.instructions, &inst);
    asmv_inst *inst_ref =
        listmv_at(&assembler.instructions, assembler.instructions.len - 1);

    for (int i = 0; i < instruction_table[inst.code].num_ops; i++)
      if (inst_ref->operands[i].type == asmv_label_type) {
        label_reference ref = {assembler.instructions.len - 1, i};
        listmv_push(&assembler.label_refs, &ref);
      }
  }

  // check labels
  for (int i = 0; i < assembler.label_addrs.len; i++) {
    asmv_label label = *(asmv_label *)listmv_at(&assembler.label_addrs, i);
    bool matched     = false;
    for (int j = 0; j < assembler.label_refs.len; j++) {
      label_reference *ref = listmv_at(&assembler.label_refs, j);
      asmv_inst *inst  = listmv_at(&assembler.instructions, ref->inst_index);
      asmv_operand *op = &inst->operands[ref->op_index];
      if (op->data.str.cap && !strcmp(label.str.data, op->data.str.data)) {
        listmv_free(&op->data.str);
        op->mode      = mode_immediate;
        op->data.unum = label.address;
        op->width     = smvm_reg64;
        op->size      = smvm_reg64;
        matched       = true;
        break;
      }
    }
    if (matched) continue;
  }

  // second pass
  for (int i = 0; i < assembler.instructions.len; i++) {
    asmv_inst inst = *(asmv_inst *)listmv_at(&assembler.instructions, i);
    if (inst.eof) break;
    if (inst.label) {
      listmv_free(&inst.str);
      continue;
    }
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
    // u8 primary_size = 4;
    u8 immediate_bytes[24] = {0};  // data/address bytes
    u64 immediate_size     = 0;

    for (int i = 0; i < num_ops; i++) {
      asmv_operand op = inst.operands[i];

      // set the info and mode bits
      primary_bytes[i] |= op.mode << 6;
      primary_bytes[1] |= op.width << (i * 2);
      if (op.type == asmv_str_type) continue;

      // set the register/data bits
      u8 byte   = i == 2 ? 3 : 2;
      u8 offset = i == 1 ? 3 : 0;
      if (op.mode == mode_register || op.mode == mode_indirect) {
        primary_bytes[byte] |= op.data.reg << offset;
      } else if (op.mode == mode_direct || op.mode == mode_immediate) {
        primary_bytes[byte] |= op.size << offset;
        u8 data_size = 1 << op.size;  // ranges from 1 to 8
        immediate_size += data_size;
        for (int n = 0; n < data_size; n++)
          immediate_bytes[immediate_size - n - 1] =
              (op.data.num >> (n * 8)) & 0xff;
      }
    }

    listmv_push_array(&vm->bytecode, primary_bytes, num_ops == 3 ? 4 : 3);
    listmv_push_array(&vm->bytecode, immediate_bytes, immediate_size);
    for (int i = 0; i < num_ops; i++) {
      asmv_operand op = inst.operands[i];
      if (op.type != asmv_str_type) continue;
      listmv_push_array(&vm->bytecode, op.data.str.data, op.data.str.len);
      listmv_free(&op.data.str);
    }
  }

  listmv_free(&assembler.instructions);
  listmv_free(&assembler.label_refs);
  listmv_free(&assembler.label_addrs);
}

/* vm - opcode functions - implementation */

void trap_fn(smvm *vm) { smvm_set_flag(vm, flag_t); }
void mov_fn(smvm *vm) {
  mov_mem((u8 *)vm->pointers[0], (u8 *)vm->pointers[1], vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void swap_fn(smvm *vm) {}
void lea_fn(smvm *vm) {}
void add_fn(smvm *vm) {
  i64 result = *vm->pointers[1] + *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void addu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->pointers[1] + *(u64 *)vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void addf_fn(smvm *vm) {}
void sub_fn(smvm *vm) {
  i64 result = *vm->pointers[1] - *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void subu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->pointers[1] - *(u64 *)vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void subf_fn(smvm *vm) {}
void mul_fn(smvm *vm) {
  i64 result = *vm->pointers[1] * *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void mulu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->pointers[1] * *(u64 *)vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void mulf_fn(smvm *vm) {}
void div_fn(smvm *vm) {
  i64 result = *vm->pointers[1] / *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void divu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->pointers[1] / *(u64 *)vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void divf_fn(smvm *vm) {}
void inc_fn(smvm *vm) {
  u64 op = *vm->pointers[0] + 1;
  mov_mem((u8 *)vm->pointers[0], (u8 *)&op, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void dec_fn(smvm *vm) {
  u64 op = *vm->pointers[0] - 1;
  mov_mem((u8 *)vm->pointers[0], (u8 *)&op, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void and_fn(smvm *vm) {
  u64 result = *vm->pointers[1] & *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void or_fn(smvm *vm) {
  u64 result = *vm->pointers[1] | *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void xor_fn(smvm *vm) {
  u64 result = *vm->pointers[1] ^ *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void shl_fn(smvm *vm) {
  u64 result = *vm->pointers[1] << *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void shr_fn(smvm *vm) {
  u64 result = *vm->pointers[1] >> *vm->pointers[2];
  mov_mem((u8 *)vm->pointers[0], (u8 *)&result, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void shli_fn(smvm *vm) {}
void shri_fn(smvm *vm) {}
void slc_fn(smvm *vm) {}
void src_fn(smvm *vm) {}
void jmp_fn(smvm *vm) {
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->pointers[0], vm->widths[0]);
}
void je_fn(smvm *vm) {
  if (*vm->pointers[0] != *vm->pointers[1]) {
    smvm_ip_inc(vm, 1);
    return;
  }  // else
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->pointers[2], vm->widths[2]);
}
void jne_fn(smvm *vm) {
  u64 left, right;
  mov_mem((u8 *)&left, (u8 *)vm->pointers[0], vm->widths[0]);
  mov_mem((u8 *)&right, (u8 *)vm->pointers[1], vm->widths[1]);
  if (left == right) {
    smvm_ip_inc(vm, vm->offset);
    return;
  }  // else
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->pointers[2], vm->widths[2]);
}
void jl_fn(smvm *vm) {}
void loop_fn(smvm *vm) {
  stack_push_u64(vm, vm->registers[reg_ip]);
  vm->registers[reg_ip] = 0;
  smvm_ip_inc(vm, vm->offset);
}
void call_fn(smvm *vm) {
  stack_push_u64(vm, vm->registers[reg_ip] + vm->offset);
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->pointers[0], vm->widths[0]);
}
void ret_fn(smvm *vm) { vm->registers[reg_ip] = stack_pop_u64(vm); }
void push_fn(smvm *vm) {}
void pop_fn(smvm *vm) {}
void sys_fn(smvm *vm) {}
void getu_fn(smvm *vm) {
  u64 input;
  scanf("%lu", &input);
  mov_mem((u8 *)vm->pointers[0], (u8 *)&input, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void puti_fn(smvm *vm) {
  i64 data;
  mov_mem((u8 *)&data, (u8 *)vm->pointers[0], vm->widths[0]);
  printf("%li\n", data);
  smvm_ip_inc(vm, vm->offset);
}
void putu_fn(smvm *vm) {
  u64 data = 0;
  mov_mem((u8 *)&data, (u8 *)vm->pointers[0], vm->widths[0]);
  printf("%lu\n", data);
  smvm_ip_inc(vm, vm->offset);
}
void putf_fn(smvm *vm) {}
void puts_fn(smvm *vm) {
  smvm_ip_inc(vm, vm->offset);
  char *str = (char *)listmv_at(&vm->bytecode, vm->registers[reg_ip]);
  u64 len   = 0;
  while (str[len] != '\0') len++;
  len++;
  printf("%s", str);
  smvm_ip_inc(vm, len);
}

/* vm - helpers - implementation */

static void stack_push_u64(smvm *vm, u64 value) {
  listmv_grow(&vm->stack, vm->stack.len + 8);
  mov_mem((u8 *)vm->stack.data, (u8 *)&value, 8);
  vm->stack.len += 8;
  update_stack_pointer(vm);
}

u64 stack_pop_u64(smvm *vm) {
  u64 value = 0;
  u8 *bytes = listmv_pop_array(&vm->stack, 8);
  update_stack_pointer(vm);
  mov_mem((u8 *)&value, bytes, 8);
  return value;
}

static bool is_little_endian() {
  uint32_t num = 1;
  return *(uint8_t *)&num == 1;
}

/*** util - implementation ***/

void listmv_init(listmv *ls, long size) {
  ls->len  = 0;
  ls->cap  = 1;
  ls->size = size;
  ls->data = calloc(ls->cap, size);
}

void listmv_push(listmv *ls, void *data) {
  if (ls->len >= ls->cap) { ls->cap *= 2; }
  ls->data = realloc(ls->data, ls->cap * ls->size);
  if (ls->data == NULL) {
    fprintf(stderr, "Memory reallocation failed in growing memory/stack.\n");
    exit(1);
  }
  memcpy((char *)ls->data + ls->len * ls->size, data, ls->size);
  ls->len++;
}

void listmv_push_array(listmv *ls, void *data, size_t num) {
  while (ls->len + num >= ls->cap) {
    ls->cap *= 2;
    ls->data = realloc((char *)ls->data, ls->cap * ls->size);
    if (ls->data == NULL) {
      fprintf(stderr, "Memory reallocation failed in growing memory/stack.\n");
      exit(1);
    }
  }
  mov_mem((u8 *)ls->data + ls->len * ls->size, (u8 *)data, num * ls->size);
  ls->len += num;
}

void *listmv_pop(listmv *ls) {
  if (ls->len == 0) return NULL;
  ls->len--;
  return (char *)ls->data + ls->len * ls->size;
}

void *listmv_pop_array(listmv *ls, u64 num) {
  if (ls->len < num) return NULL;
  ls->len -= num;
  return (char *)ls->data + ls->len * ls->size;
}

void listmv_grow(listmv *ls, u64 new_cap) {
  if (new_cap <= ls->cap) return;
  ls->cap  = new_cap;
  ls->data = realloc(ls->data, ls->cap * ls->size);
  if (ls->data == NULL) {
    fprintf(stderr, "Memory reallocation failed in growing memory/stack.\n");
    exit(1);
  }
}

void *listmv_at(listmv *ls, u64 index) {
  return (char *)ls->data + index * ls->size;
}

void listmv_free(listmv *ls) {
  free(ls->data);
  ls->cap = 0;
}

static void print_memory(void *arr, int num) {
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

#endif
#endif
