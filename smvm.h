#ifndef smv_smvm_impl_h
#define smv_smvm_impl_h

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// registers are 64-bit
// so is all of memory
// I am generally very paranoid of stuff like this but for once in
// my life I won't worry about it and just use fixed size registers
typedef uint64_t u64;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;

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
  listmv allocation;
  listmv bytecode;
  u64 registers[smvm_register_num];
  // 000 - ip: instruction pointer
  // 001 - a:  NOT accumulator, rather a
  // 010 - b:  just b
  // 011 - c:  counter register
  // 100 - d:  destination register
  u16 flags;
} smvm;

typedef enum smvm_opcode {
  // opcode takes 6 bits
  op_halt = 0b00000,  // halts the program
  op_mov = 0b00001,   // mov x y
  op_add = 0b00010,   // add x y
  op_addi = 0b00011,  // addi x y
  op_addf = 0b00100,  // addf x y
  op_sub = 0b00101,   // sub x y
  op_subi = 0b00110,  // subi x y
  op_subf = 0b00111,  // subf x y
  op_mul = 0b01000,   // mul x y
  op_muli = 0b01001,  // muli x y
  op_mulf = 0b01010,  // mulf x y
  op_div = 0b01011,   // div x y
  op_divi = 0b01100,  // divi x y
  op_divf = 0b01101,  // divf x y
  op_jmp = 0b01110,   // jmp (to [d])
  op_je = 0b01111,    // if (a == b) jmp (to [d])
  op_jne = 0b10000,   // if (a != b) jmp (to [d])
  op_jl = 0b10001,    // if (a < b) jmp (to [d])
  op_loop = 0b10010,  // if (c > 0) dec c ; jmp (to [d])
  op_inc = 0b10011,   // inc x
  op_dec = 0b10100,   // dec x
  // NOTE: 1_____ > print instructions
  // these might be temporary instructions
  op_print_int = 0b10101,
  op_print_inti = 0b10110,
  op_print_float = 0b10111,
  op_print_str = 0b11000,  // prints till it encounters 0x0000
} smvm_opcode;

typedef enum smvm_register {
  reg_a = 0b000,
  reg_b = 0b001,
  reg_c = 0b010,
  reg_d = 0b011,
  reg_ip = 0b100,
} smvm_register;

typedef enum smvm_flag {
  flag_o = 1,       // overflow
  flag_d = 1 << 1,  // direction
  flag_t = 1 << 2,  // trap ~TODO add errors~
  flag_s = 1 << 3,  // sign
  flag_z = 1 << 4,  // zero
} smvm_flag;

typedef enum smvm_inst_mode {
  // mode takes 2 bits
  mode_register = 0b00,   // r
  mode_immediate = 0b01,  // i
  mode_indirect = 0b10,   // a
  mode_direct = 0b11,     // d
  mode_implicit = 0b1111,
  // implicit type is quite literally, implicit; implicit instructions are
  // simply run without checking the "mode" first, unlike other instructions
} smvm_inst_mode;

/* this is the struct a user should be conerned with */
typedef struct smvm_inst {
  smvm_opcode code;
  smvm_inst_mode mode;
  u64 x;
  u64 y;
  u64 z;
} smvm_inst;

void smvm_init(smvm *vm);
void smvm_load_inst(smvm *vm, smvm_inst *inst, long num);
// void smvm_load_bytecode(smvm *vm, u8 *code, long num); // TODO
void smvm_execute(smvm *vm);
void smvm_free(smvm *vm);

/* vm - helpers */
u64 smvm_fetch_u64(smvm *vm);  // fetches 8 byte in native endian
// fetches operands and performs operations
static void smvm_handle_op(smvm *vm, u8 byte0,
                           void (*op_func)(smvm *, u8, u8, u8));
// converts to native endian from little-endian
void *convert_endian(void *arr, int num);
static void update_flags(smvm *vm, u64 result);

/*** vm - inline helpers ***/

static inline void smvm_ip_inc(smvm *vm) { vm->registers[reg_ip]++; }

static inline u8 *smvm_fetch_inst_addr(smvm *vm) {
  return listmv_at(&vm->bytecode, vm->registers[reg_ip]);
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
  // TODO, see if this can become 53-bit somehow
  listmv_init(&vm->allocation, sizeof(u64));

  for (int i = 0; i < smvm_register_num; i++) vm->registers[i] = 0;
}

void smvm_load_inst(smvm *vm, smvm_inst *inst, long num) {
  for (size_t i = 0; i < num; i++) {
    smvm_inst c = inst[i];
    u8 operand = (c.code << 2) | c.mode;

    // refer to [docs/BYTECODE.md]
    switch (c.mode) {
      case mode_indirect:
      case mode_register: {
        u8 inst[2] = {
            ((c.x & 0b111) << 7) | operand,
            ((c.x & 0b111) << 6) | ((c.y & 0b111) << 3) | (c.z & 0b111),
        };
        listmv_push_array(&vm->bytecode, &inst, 2);
        break;
      }

      case mode_direct:
      case mode_immediate: {
        u8 long_inst[10] = {
            operand,
            (c.x << 3) | c.y,
        };
        for (int j = 0; j < 8; j++) {
          long_inst[2 + j] = (c.z >> (8 * j)) & 0xff;
        }
        listmv_push_array(&vm->bytecode, &long_inst, 10);
        break;
      }

      case mode_implicit: {
        u16 inst = c.code;
        listmv_push(&vm->bytecode, &inst);
        break;
      }

      default:  // TODO error handler
        break;
    }
  }
}

/* NOTE these aren't inline helpers! these are the real deal */
static inline void add_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = vm->registers[regy] + vm->registers[regz];
  update_flags(vm, vm->registers[regx]);
}

static inline void addi_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = (i64)vm->registers[regy] + (i64)vm->registers[regz];
  update_flags(vm, vm->registers[regx]);
}

static inline void addf_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  double *dest = (double *)&vm->registers[regx];
  double *src1 = (double *)&vm->registers[regy];
  double *src2 = (double *)&vm->registers[regz];
  *dest = *src1 + *src2;
}

static inline void sub_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = vm->registers[regy] - vm->registers[regz];
  update_flags(vm, vm->registers[regx]);
}

static inline void subi_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = (i64)vm->registers[regy] - (i64)vm->registers[regz];
  update_flags(vm, vm->registers[regx]);
}

static inline void subf_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  double *dest = (double *)&vm->registers[regx];
  double *src1 = (double *)&vm->registers[regy];
  double *src2 = (double *)&vm->registers[regz];
  *dest = *src1 - *src2;
}

static inline void mul_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = vm->registers[regy] * vm->registers[regz];
  update_flags(vm, vm->registers[regx]);
}

static void muli_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = (i64)vm->registers[regy] * (i64)vm->registers[regz];
  update_flags(vm, vm->registers[regx]);
}

static void mulf_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  double *dest = (double *)&vm->registers[regx];
  double *src1 = (double *)&vm->registers[regy];
  double *src2 = (double *)&vm->registers[regz];
  *dest = *src1 * *src2;
}

static void div_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = vm->registers[regy] / vm->registers[regz];
  update_flags(vm, vm->registers[regx]);
}

static void divi_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = (i64)vm->registers[regy] / (i64)vm->registers[regz];
  update_flags(vm, vm->registers[regx]);
}

static void divf_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  double *dest = (double *)&vm->registers[regx];
  double *src1 = (double *)&vm->registers[regy];
  double *src2 = (double *)&vm->registers[regz];
  *dest = *src1 / *src2;
}

static void mov_op(smvm *vm, u8 regx, u8 regy, u8 regz) {
  vm->registers[regx] = vm->registers[regy];
}

// realer deal (maybe not)
// TODO error return type
void smvm_execute(smvm *vm) {
  while (true) {
    u8 *ptr = smvm_fetch_inst_addr(vm);
    u8 byte0 = *ptr;
    smvm_ip_inc(vm);
    smvm_opcode code = byte0 >> 2;

    switch (code) {
      case op_halt:
        return;

      case op_add:
        smvm_handle_op(vm, byte0, add_op);
        break;

      case op_addi:
        smvm_handle_op(vm, byte0, addi_op);
        break;

      case op_addf:
        smvm_handle_op(vm, byte0, addf_op);
        break;

      case op_sub:
        smvm_handle_op(vm, byte0, sub_op);
        break;

      case op_subi:
        smvm_handle_op(vm, byte0, subi_op);
        break;

      case op_subf:
        smvm_handle_op(vm, byte0, subf_op);
        break;

      case op_mul:
        smvm_handle_op(vm, byte0, mul_op);
        break;

      case op_muli:
        smvm_handle_op(vm, byte0, muli_op);
        break;

      case op_mulf:
        smvm_handle_op(vm, byte0, mulf_op);
        break;

      case op_div:
        smvm_handle_op(vm, byte0, div_op);
        break;

      case op_divi:
        smvm_handle_op(vm, byte0, divi_op);
        break;

      case op_divf:
        smvm_handle_op(vm, byte0, divf_op);
        break;

      case op_mov:
        smvm_handle_op(vm, byte0, mov_op);
        break;

      case op_print_int: {
        u8 mode = byte0 & 0b11;
        if (mode == mode_register) {
          u8 reg = (byte0 >> 5);
          printf("%lu\n", vm->registers[reg]);
        }
        smvm_ip_inc(vm);
        break;
      }

      case op_print_str: {
        u8 mode = byte0 & 0b11;
        if (mode == mode_register) {
          u8 reg = (byte0 >> 5);
          printf("%s\n", (char *)listmv_at(&vm->memory, vm->registers[reg]));
        }
        smvm_ip_inc(vm);
        break;
      }

      default:  // error
        break;
    }
  }
}

void smvm_free(smvm *vm) {
  listmv_free(&vm->memory);
  listmv_free(&vm->allocation);
  listmv_free(&vm->bytecode);
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

static void smvm_handle_op(smvm *vm, u8 byte0,
                           void (*op_func)(smvm *, u8, u8, u8)) {
  smvm_ip_inc(vm);
  u8 byte1 = *smvm_fetch_inst_addr(vm);
  u8 regx = (byte0 >> 5) & 0b111;
  u8 regy = (byte1 >> 3) & 0b111;
  u8 regz = byte1 & 0b111;
  smvm_ip_inc(vm);
  op_func(vm, regx, regy, regz);
}

static void update_flags(smvm *vm, u64 result) {
  if (result == 0) {
    smvm_set_flag(vm, flag_z);
  } else {
    smvm_reset_flag(vm, flag_z);
  }
  if (result >> 63) {
    smvm_set_flag(vm, flag_s);
  } else {
    smvm_reset_flag(vm, flag_s);
  }
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
