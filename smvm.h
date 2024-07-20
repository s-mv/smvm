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

#define listmv(_type) listmv

void listmv_init(listmv *ls, long size);
void listmv_push(listmv *ls, void *data);
void listmv_push_array(listmv *ls, void *data, size_t num);
void *listmv_pop(listmv *ls);
void *listmv_pop_array(listmv *ls, u64 num);
void listmv_grow(listmv *ls, u64 new_cap);
void *listmv_at(listmv *ls, u64 index);
void listmv_free(listmv *ls);
i64 parse_signed(u64 unum, u8 width) {
  if (width == 8) return unum;
  i64 result;
  u64 mask = (1ull << (width * 8)) - 1;
  u64 sign_bit = 1ull << ((width * 8) - 1);

  unum &= mask;
  if (unum & sign_bit) result = (i64)(unum | ~mask);
  else result = (i64)unum;
  return result;
}
// just a helper for better debugging
static void print_memory(void *arr, int num);

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
  op_mov = 0b000001,
  op_movu = 0b000010,
  op_movf = 0b000011,
  op_swap = 0b000100,
  op_lea = 0b000101,
  op_add = 0b000110,
  op_addu = 0b000111,
  op_addf = 0b001000,
  op_sub = 0b001001,
  op_subu = 0b001010,
  op_subf = 0b001011,
  op_mul = 0b001100,
  op_mulu = 0b001101,
  op_mulf = 0b001110,
  op_div = 0b001111,
  op_divu = 0b10000,
  op_divf = 0b10001,
  op_inc = 0b10010,
  op_dec = 0b10011,
  op_and = 0b10100,
  op_or = 0b10101,
  op_xor = 0b10110,
  op_shl = 0b10111,
  op_shr = 0b11000,
  op_shli = 0b11001,
  op_shri = 0b11010,
  op_slc = 0b11011,
  op_src = 0b11100,
  op_jmp = 0b11101,
  op_je = 0b11110,
  op_jne = 0b11111,
  op_jl = 0b100000,
  op_loop = 0b100001,
  op_call = 0b100010,
  op_ret = 0b100011,
  op_push = 0b100100,
  op_pop = 0b100101,
  op_scall = 0b100110,
  op_getu = 0b100111,
  op_puti = 0b101000,
  op_putu = 0b101001,
  op_putf = 0b101010,
  op_puts = 0b101011,
} smvm_opcode;

typedef enum smvm_register {
  reg_a = 0b000,
  reg_b = 0b001,
  reg_c = 0b010,
  reg_d = 0b011,
  reg_e = 0b100,
  reg_sp = 0b101,
  reg_ip = 0b110,
  reg_none = 0b1000
} smvm_register;

typedef enum smvm_data_width {
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
  mode_indirect = 0b01,    // x
  mode_immediate = 0b10,   // i
  mode_direct = 0b11,      // d
  mode_implicit = 0b1111,  // TODO remove?
  // implicit type is quite literally, implicit; implicit instructions are
  // simply run without checking the "mode" first, unlike other instructions
} smvm_mode;

void smvm_init(smvm *vm);
void smvm_assemble(smvm *vm, char *code);
void smvm_execute(smvm *vm);
void smvm_disassemble(smvm *vm, char *code);
void smvm_free(smvm *vm);

/* vm - helpers */

// converts to native endian from little-endian, TODO
static void stack_push(smvm *vm, u8 *value, u64 width);
static u8 *stack_pop(smvm *vm, u64 width);
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
void movu_fn(smvm *vm);
void movf_fn(smvm *vm);
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
    [op_halt] = {"halt", 4, 0, trap_fn},  [op_mov] = {"mov", 3, 2, mov_fn},
    [op_movu] = {"movu", 4, 2, movu_fn},  [op_movf] = {"movf", 4, 2, movf_fn},
    [op_swap] = {"swap", 4, 2, swap_fn},  [op_lea] = {"lea", 3, 2, lea_fn},
    [op_addu] = {"addu", 4, 3, addu_fn},  [op_addf] = {"addf", 4, 3, addf_fn},
    [op_add] = {"add", 3, 3, add_fn},     [op_subu] = {"subu", 4, 3, subu_fn},
    [op_subf] = {"subf", 4, 3, subf_fn},  [op_sub] = {"sub", 3, 3, sub_fn},
    [op_mulu] = {"mulu", 4, 3, mulu_fn},  [op_mulf] = {"mulf", 4, 3, mulf_fn},
    [op_mul] = {"mul", 3, 3, mul_fn},     [op_divu] = {"divu", 4, 3, divu_fn},
    [op_divf] = {"divf", 4, 3, divf_fn},  [op_div] = {"div", 3, 3, div_fn},
    [op_inc] = {"inc", 3, 1, inc_fn},     [op_dec] = {"dec", 3, 1, dec_fn},
    [op_and] = {"and", 3, 3, and_fn},     [op_or] = {"or", 2, 3, or_fn},
    [op_xor] = {"xor", 3, 3, xor_fn},     [op_shli] = {"shli", 4, 3, shli_fn},
    [op_shl] = {"shl", 3, 3, shl_fn},     [op_shri] = {"shri", 4, 3, shri_fn},
    [op_shr] = {"shr", 3, 3, shr_fn},     [op_slc] = {"slc", 3, 3, slc_fn},
    [op_src] = {"src", 3, 3, src_fn},     [op_jmp] = {"jmp", 3, 1, jmp_fn},
    [op_je] = {"je", 2, 3, je_fn},        [op_jne] = {"jne", 3, 3, jne_fn},
    [op_jl] = {"jl", 2, 3, jl_fn},        [op_loop] = {"loop", 4, 2, loop_fn},
    [op_call] = {"call", 4, 1, call_fn},  [op_ret] = {"ret", 3, 0, ret_fn},
    [op_push] = {"push", 4, 1, push_fn},  [op_pop] = {"pop", 3, 1, pop_fn},
    [op_scall] = {"scall", 5, 1, sys_fn}, [op_getu] = {"getu", 4, 1, getu_fn},
    [op_puti] = {"puti", 4, 1, puti_fn},  [op_putu] = {"putu", 4, 1, putu_fn},
    [op_putf] = {"putf", 4, 1, putf_fn},  [op_puts] = {"puts", 4, 1, puts_fn}};

#define instruction_table_len \
  (sizeof(instruction_table) / sizeof(*instruction_table))

/*** vm - assembler (asmv) ***/

typedef struct asmv {
  char *code;  // input
  listmv instructions;
  listmv label_refs;
  listmv label_addrs;
  // output of type listmv(u8 *)
  listmv bytecode;
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
  smvm_mode mode;
  smvm_data_width width : 4;
  smvm_data_width size : 4;
  asmv_op_data data;
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
  u64 index;
} asmv_inst;

typedef struct asmv_label {
  listmv str;
  u64 address;
} asmv_label;

typedef struct label_reference {
  u64 inst_index;
  u8 op_index;
} label_reference;

/* asmv - inline functions */
static inline u64 asmv_current(asmv *a) { return a->code[a->index]; }
static inline u64 asmv_peek(asmv *a) { return a->code[a->index + 1]; }
static inline u64 asmv_peek2(asmv *a) { return a->code[a->index + 2]; }
static inline u64 asmv_prev(asmv *a) { return a->code[a->index - 1]; }
static inline u64 asmv_prev2(asmv *a) { return a->code[a->index - 2]; }
static inline u64 asmv_next(asmv *a) { return a->code[++a->index]; }
static inline void asmv_skip(asmv *a) { ++a->index; }

/* asmv - function delcaration */
static void asmv_init(asmv *as);
static void asmv_assemble(asmv *as);
static asmv_inst asmv_lex_inst(asmv *as);
static void asmv_free(asmv *as);

// helper function
static u8 asmv_parse_register(asmv *as);

/*** vm - disassembler (dsmv) ***/

typedef struct dsmv {
  listmv bytecode;  // input, listmv(u8)
  listmv code;      // output, listmv(str)
  u64 index;
} dsmv;

static void dsmv_init(dsmv *ds);
static void dsmv_disassemble();
/*** vm - disassembler (dsmv) - implementation ***/

static void dsmv_init(dsmv *ds) {
  listmv_init(&ds->code, sizeof(char));
  ds->index = 0;
}

static void dsmv_disassemble() {
  // TODO
}

/* vm - disassembler (dsmv) - inline functions */
static inline void dsmv_free(dsmv *ds) {
  listmv_free(&ds->code);
  // ownership of ds->bytecode goes to a vm
  // so no need to free it
}

// ---

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

void smvm_assemble(smvm *vm, char *code) {
  asmv assembler;
  asmv_init(&assembler);
  assembler.code = code;
  asmv_assemble(&assembler);
  if (vm->bytecode.data != NULL) listmv_free(&vm->bytecode);
  vm->bytecode = assembler.bytecode;  // ownership to vm
  asmv_free(&assembler);
}

// TODO error return type
void smvm_execute(smvm *vm) {
  while (true) {
    u8 *inst = smvm_fetch_inst_addr(vm);
    u8 code = inst[0] & 0x3f;
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
            u8 size = 1 << (reg[i] & 3);
            u64 address = 0;
            // reg also holds size
            mov_mem((u8 *)&address, inst + offset, size);
            listmv_grow(&vm->memory, address + vm->widths[i] + 1);
            offset += size;
            vm->pointers[i] = listmv_at(&vm->memory, address);
            break;
          }
          case mode_immediate:
            u8 size = 1 << (reg[i] & 3);
            vm->data[i] = 0;
            if (vm->little_endian)
              mov_mem_reverse((u8 *)(&vm->data[i]), inst + offset, size);
            else mov_mem((u8 *)(&vm->data[i]), inst + offset, size);
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

void smvm_disassemble(smvm *vm, char *code) {
  dsmv dis;
  dsmv_init(&dis);
  dis.bytecode = vm->bytecode;
  dsmv_disassemble();
  // code = malloc(dis.code.len);
  strncpy(code, (char *)dis.code.data, dis.code.len);
  dsmv_free(&dis);
}

/*** vm - assembler - implementation ***/

static void asmv_init(asmv *as) {
  as->index = 0;
  as->panic_mode = false;
  listmv_init(&as->bytecode, sizeof(u8));
  listmv_init(&as->instructions, sizeof(asmv_inst));
  listmv_init(&as->label_addrs, sizeof(asmv_label));
  listmv_init(&as->label_refs, sizeof(label_reference));
}

static u8 asmv_parse_register(asmv *as) {
  u64 backup_index = as->index;

  if (asmv_current(as) == 'i' && asmv_peek(as) == 'p' &&
      !isalnum(asmv_peek2(as))) {
    as->index += 2;
    return (smvm_reg64 << 3) | reg_ip;
  }

  if (asmv_current(as) == 's' && asmv_peek(as) == 'p' &&
      !isalnum(asmv_peek2(as))) {
    as->index += 2;
    return (smvm_reg64 << 3) | reg_sp;
  }

  if (asmv_current(as) == 'a' || asmv_current(as) == 'b' ||
      asmv_current(as) == 'c' || asmv_current(as) == 'd') {
    asmv_skip(as);
    if (asmv_current(as) == '8') {
      asmv_skip(as);
      return (smvm_reg8 << 3) | (reg_a + asmv_prev(as) - 'a');
    }
    if (asmv_current(as) == '1' && asmv_peek(as) == '6') {
      as->index += 2;
      return (smvm_reg16 << 3) | (reg_a + asmv_prev2(as) - 'a');
    }
    if (asmv_current(as) == '3' && asmv_peek(as) == '2') {
      as->index += 2;
      return (smvm_reg32 << 3) | (reg_a + asmv_prev2(as) - 'a');
    }
    if (asmv_current(as) == '6' && asmv_peek(as) == '4') {
      as->index += 2;
      return (smvm_reg64 << 3) | (reg_a + asmv_prev2(as) - 'a');
    }

    if (!isdigit(asmv_current(as)))
      return (smvm_reg64 << 3) | (reg_a + asmv_prev(as) - 'a');
  }

  as->index = backup_index;
  return reg_none;
}

// TODO, floating point numbers
static asmv_op_data parse_number(asmv *as) {
  u64 num = 0;
  bool neg = false;
  if (asmv_current(as) == '-') {
    neg = true;
    asmv_skip(as);
  }

  while (isdigit(asmv_current(as))) {
    num = num * 10 + (asmv_current(as) - '0');
    asmv_skip(as);
    if (asmv_current(as) == '_')
      asmv_skip(as);  // something like 1_00_000 is valid
  }
  asmv_op_data data;
  if (neg) {
    data.type = asmv_num_type;
    data.num = -num;
  } else {
    data.type = asmv_unum_type;
    data.unum = num;
  }
  return data;
}

// I hate nesting
static asmv_inst asmv_lex_inst(asmv *as) {
  char buffer[512];
  u64 offset = 0;
  asmv_operand op = {0};
  asmv_inst inst = {.error = asmv_all_ok, .eof = false, .label = false};
  char current;

  while (isspace(asmv_current(as))) asmv_skip(as);
  current = asmv_current(as);
  // ignore comments
  if (current == ';' || current == '#')
    while (asmv_next(as) != '\n');
  while (isspace(asmv_current(as))) asmv_skip(as);
  current = asmv_current(as);
  if (current == '\0') return ((asmv_inst){.eof = true});
  // label
  if (current == '.') {
    asmv_skip(as);
    offset = 0;
    while (isalnum(as->code[as->index + offset])) offset++;
    char eof = '\0';
    listmv_init(&inst.str, sizeof(char));
    listmv_push_array(&inst.str, as->code + as->index, offset);
    listmv_push(&inst.str, &eof);
    inst.label = true;
    inst.index = as->index;
    as->index += offset;
    return inst;
  }
  current = asmv_current(as);
  if (!isalpha(current)) return (asmv_inst){.error = asmv_misc_error};

  while (isalpha(as->code[as->index + offset])) offset++;
  strncpy(buffer, as->code + as->index, offset);
  buffer[offset + 1] = '\0';
  as->index += offset;

  for (int i = 0; i < instruction_table_len; i++) {
    if (strncmp(instruction_table[i].name, buffer,
                instruction_table[i].str_size))
      continue;

    inst.code = i;

    for (int j = 0; j < instruction_table[i].num_ops; j++) {
      while (isspace(asmv_current(as))) asmv_skip(as);
      current = asmv_current(as);

      if (current == '-' || isdigit(current)) {
        // immediate mode, handle numbers
        // TODO: handle bases and .d+ (clashes with labels right now)
        // sub 12 b c makes no sense, 12 = b + c what??
        if (instruction_table[i].num_ops > 1 && j == 0)
          return (asmv_inst){.error = asmv_immediate_x};
        // handle immediate addressing
        // either u or f, otherwise irrelevant

        op.mode = mode_immediate;
        asmv_op_data data = parse_number(as);
        op.data = data;
        if (data.type == asmv_num_type) {
          op.size = min_space_needed(op.data.num);
          if (op.size != smvm_reg64)
            op.data.num = op.data.num & ((1ull << (8 << op.size)) - 1);
        } else if (data.type == asmv_unum_type) {
          op.size = min_space_neededu(op.data.num);
        }
        op.width = op.size;
        // TODO, floating point numbers
      } else if (current == '@') {
        asmv_skip(as);
        while (isspace(asmv_current(as))) asmv_skip(as);
        if (isdigit(asmv_current(as))) {
          // if number, direct addressing mode
          // we don't check for - here since addresses are non-negative
          op.mode = mode_direct;
          op.data = parse_number(as);
          op.size = min_space_neededu(op.data.unum);
          if (asmv_current(as) == '>') {
            asmv_skip(as);
            if (asmv_current(as) == '6' && asmv_peek(as) == '4') {
              op.width = smvm_reg64;
              as->index += 2;
            } else if (asmv_current(as) == '3' && asmv_peek(as) == '2') {
              op.width = smvm_reg32;
              as->index += 2;
            } else if (asmv_current(as) == '1' && asmv_peek(as) == '6') {
              op.width = smvm_reg16;
              as->index += 2;
            } else if (asmv_current(as) == '8') {
              op.width = smvm_reg8;
              asmv_skip(as);
            }
          } else op.width = smvm_reg64;

        } else if (current == 'r') {
          // else if register, indirect addressing mode
          asmv_skip(as);
          smvm_register reg = asmv_parse_register(as);
          // TODO handle reg_none better
          if (reg == reg_none) break;
          op.mode = mode_indirect;
          op.width = reg >> 3;
          op.data.reg = reg & 0b111;
        } else {
          // TODO, handle error
        }
      } else if (current == 'r') {
        asmv_skip(as);
        smvm_register reg = asmv_parse_register(as);  // gives width|reg
        // TODO handle reg_none better mayhaps
        // time will tell
        if (reg == reg_none) continue;
        op.mode = mode_register;
        op.width = reg >> 3;
        op.data.reg = reg & 0b111;
      } else if (current == '"' && inst.code == op_puts) {
        asmv_skip(as);
        listmv_init(&op.data.str, sizeof(char));

        while (asmv_current(as) != '"') {
          current = asmv_current(as);
          // checks for `\n`, etc.
          if (current == '\\' && asmv_peek(as) != '\0') {
            current = asmv_next(as);
            switch (current) {
              case 'n': current = '\n'; break;
              case 't': current = '\t'; break;
              case 'r': current = '\r'; break;
              case '\\':
              case '"': break;
              default: as->index--; current = '\\';
            }
          }
          listmv_push(&op.data.str, &current);
          asmv_skip(as);
        }

        listmv_push(&op.data.str, &(char){'\0'});

        op.data.type = asmv_str_type;
        op.mode = mode_register;
        asmv_skip(as);
      } else if (current == '.') {  // handling labels
        asmv_skip(as);
        offset = 0;
        while (isalnum(as->code[as->index + offset])) offset++;
        op.data.type = asmv_label_type;
        op.mode = mode_immediate;
        char eof = '\0';
        listmv_init(&op.data.str, sizeof(char));
        listmv_push_array(&op.data.str, as->code + as->index, offset);
        listmv_push(&op.data.str, &eof);
        as->code += offset;
      } else {
        // TODO edge cases, error handling
      }
      inst.operands[j] = op;
    }
    break;
  }
  return inst;
}

static void asmv_assemble(asmv *as) {
  u64 offset = 0;  // track offset

  // first pass
  while (as->code[as->index] != '\0') {
    asmv_inst inst = asmv_lex_inst(as);
    if (inst.eof) break;
    if (!inst.label) {
      if (instruction_table[inst.code].num_ops == 0) offset++;
      else {
        offset += instruction_table[inst.code].num_ops == 3 ? 4 : 3;
        for (int i = 0; i < instruction_table[inst.code].num_ops; i++) {
          // TODO efficiency for storing labels
          if (inst.operands[i].data.type == asmv_label_type) offset += 8;
          else if (inst.operands[i].data.type == asmv_str_type)
            offset += inst.operands[i].data.str.len;
          else if (inst.operands[i].mode > 1)
            offset += 1 << inst.operands[i].size;
        }
      }
    } else {
      asmv_label label = {.address = offset, .str = inst.str};
      listmv_push(&as->label_addrs, &label);
    }

    listmv_push(&as->instructions, &inst);
    asmv_inst *inst_ref =
        listmv_at(&as->instructions, as->instructions.len - 1);

    for (int i = 0; i < instruction_table[inst.code].num_ops; i++)
      if (inst_ref->operands[i].data.type == asmv_label_type) {
        label_reference ref = {as->instructions.len - 1, i};
        listmv_push(&as->label_refs, &ref);
      }
  }

  // check labels
  for (int i = 0; i < as->label_addrs.len; i++) {
    asmv_label label = *(asmv_label *)listmv_at(&as->label_addrs, i);
    bool matched = false;
    for (int j = 0; j < as->label_refs.len; j++) {
      label_reference *ref = listmv_at(&as->label_refs, j);
      asmv_inst *inst = listmv_at(&as->instructions, ref->inst_index);
      asmv_operand *op = &inst->operands[ref->op_index];
      if (op->data.str.cap && !strcmp(label.str.data, op->data.str.data)) {
        listmv_free(&op->data.str);
        op->mode = mode_immediate;
        op->data.unum = label.address;
        op->width = smvm_reg64;
        op->size = smvm_reg64;
        matched = true;
        break;
      }
    }
    if (matched) continue;
  }

  // second pass
  for (int i = 0; i < as->instructions.len; i++) {
    asmv_inst inst = *(asmv_inst *)listmv_at(&as->instructions, i);
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
      listmv_push(&as->bytecode, &inst.code);
      continue;
    }

    u8 primary_bytes[4] = {inst.code, 0, 0, 0};
    // u8 primary_size = 4;
    u8 immediate_bytes[24] = {0};  // data/address bytes
    u64 immediate_size = 0;

    for (int i = 0; i < num_ops; i++) {
      asmv_operand op = inst.operands[i];

      // set the info and mode bits
      primary_bytes[i] |= op.mode << 6;
      primary_bytes[1] |= op.width << (i * 2);
      if (op.data.type == asmv_str_type) continue;

      // set the register/data bits
      u8 byte = i == 2 ? 3 : 2;
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

    listmv_push_array(&as->bytecode, primary_bytes, num_ops == 3 ? 4 : 3);
    listmv_push_array(&as->bytecode, immediate_bytes, immediate_size);
    for (int i = 0; i < num_ops; i++) {
      asmv_operand op = inst.operands[i];
      if (op.data.type != asmv_str_type) continue;
      listmv_push_array(&as->bytecode, op.data.str.data, op.data.str.len);
      listmv_free(&op.data.str);
    }
  }
}

static void asmv_free(asmv *as) {
  listmv_free(&as->instructions);
  listmv_free(&as->label_refs);
  listmv_free(&as->label_addrs);
  // bytecode is now owned by a vm
  // so no need to free it
}

/* vm - opcode functions - implementation */

void trap_fn(smvm *vm) { smvm_set_flag(vm, flag_t); }
void mov_fn(smvm *vm) {
  mov_mem((u8 *)vm->pointers[0], (u8 *)vm->pointers[1], vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void movu_fn(smvm *vm) {
  mov_mem((u8 *)vm->pointers[0], (u8 *)vm->pointers[1], vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void movf_fn(smvm *vm) {
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
  i64 left = 0, right = 0;
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
  stack_push(vm, (u8 *)&vm->registers[reg_ip], 8);
  vm->registers[reg_ip] = 0;
  smvm_ip_inc(vm, vm->offset);
}
void call_fn(smvm *vm) {
  u64 addr = vm->registers[reg_ip] + vm->offset;
  stack_push(vm, (u8 *)&addr, 8);
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->pointers[0], vm->widths[0]);
}
void ret_fn(smvm *vm) { vm->registers[reg_ip] = *(i64 *)stack_pop(vm, 8); }
void push_fn(smvm *vm) {
  stack_push(vm, (u8 *)vm->pointers[0], vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void pop_fn(smvm *vm) {
  u8 *data = stack_pop(vm, vm->widths[0]);
  mov_mem((u8 *)vm->pointers[0], data, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void sys_fn(smvm *vm) {}
void getu_fn(smvm *vm) {
  u64 input;
  scanf("%lu", &input);
  mov_mem((u8 *)vm->pointers[0], (u8 *)&input, vm->widths[0]);
  smvm_ip_inc(vm, vm->offset);
}
void puti_fn(smvm *vm) {
  i64 data = parse_signed(*vm->pointers[0], vm->widths[0]);
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
  u64 len = 0;
  while (str[len] != '\0') len++;
  len++;
  printf("%s", str);
  fflush(stdout);
  smvm_ip_inc(vm, len);
}

/* vm - helpers - implementation */

static void stack_push(smvm *vm, u8 *value, u64 width) {
  listmv_grow(&vm->stack, vm->stack.len + width);
  mov_mem((u8 *)vm->stack.data, value, width);
  vm->stack.len += width;
  update_stack_pointer(vm);
}

u8 *stack_pop(smvm *vm, u64 width) {
  u8 *bytes = listmv_pop_array(&vm->stack, width);
  update_stack_pointer(vm);
  return bytes;
}

static bool is_little_endian() {
  uint32_t num = 1;
  return *(uint8_t *)&num == 1;
}

/*** util - implementation ***/

void listmv_init(listmv *ls, long size) {
  ls->len = 0;
  ls->cap = 1;
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
  ls->cap = new_cap;
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
      if (j % 8 == 0) printf("|");
      else if (j % 4 == 0) printf(" ");
    }
  }
  printf("\n");
}

#endif
#endif
