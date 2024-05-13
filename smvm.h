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
typedef uint64_t smvm_size;
typedef uint8_t smvm_byte;
typedef uint16_t smvm_word;

/* util */

typedef struct listmv {
  void *data;
  smvm_size len;
  smvm_size cap;
  long size;
} listmv;

void listmv_init(listmv *ls, long size);
void listmv_push(listmv *ls, void *data);
void *listmv_at(listmv *ls, smvm_size index);
void listmv_free(listmv *ls);

// temp helper
void print_memory(void *arr, int num) {
  unsigned char *p = (unsigned char *)arr;
  for (size_t i = 0; i < num; i++) printf("%02x ", p[i]);
  printf("\n");
}

/* vm */

#define smvm_register_num (5)

typedef struct smvm {
  listmv memory;
  listmv allocation;
  listmv bytecode;
  smvm_size reg[smvm_register_num];
  // 000 - ip: instruction pointer
  // 001 - a:  not accumulator, rather a
  // 010 - b:  just b
  // 011 - c:  counter register
  // 100 - d:  destination register
} smvm;

typedef enum smvm_opcode {
  // opcode takes 6 bits
  op_halt = 0b000000,  // halts the program
  op_mov = 0b000001,   // mov x y
  op_add = 0b000010,   // add x y
  op_sub = 0b000011,   // sub x y
  op_jmp = 0b000100,   // jmp (to [d])
  op_je = 0b000101,    // if (a == b) jmp (to [d])
  op_jne = 0b000110,   // if (a != b) jmp (to [d])
  op_jl = 0b000111,    // if (a < b) jmp (to [d])
  op_loop = 0b001000,  // if (c > 0) dec c ; jmp (to [d])
  op_inc = 0b001010,
  op_dec = 0b001001,
  // NOTE: 1_____ > print instructions
  // these might be temporary instructio#ns
  op_print_int = 0b100000,
  op_print_float = 0b100001,
  op_print_str = 0b100010,  // prints till it encounters 0x0000
} smvm_opcode;

typedef enum smvm_reg_operand {
  reg_a = 0b000,
  reg_b = 0b001,
  reg_c = 0b010,
  reg_d = 0b011,
  reg_ip = 0b100,
} smvm_reg_operand;

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
  smvm_size x;
  smvm_size y;
} smvm_inst;

void smvm_init(smvm *vm);
void smvm_load_inst(smvm *vm, smvm_inst *inst, long num);
// void smvm_load_bytecode(smvm *vm, smvm_byte *code, long num); // TODO
void smvm_execute_inst(smvm *vm);
void smvm_free(smvm *vm);

// some helpers
smvm_word smvm_next(smvm *vm);

/* vm - implementation */

void smvm_init(smvm *vm) {
  listmv_init(&vm->bytecode, sizeof(smvm_word));
  listmv_init(&vm->memory, sizeof(smvm_size));
  // TODO, see if this can become 53-bit somehow
  listmv_init(&vm->allocation, sizeof(smvm_size));

  for (int i = 0; i < smvm_register_num; i++) vm->reg[i] = 0;
}

void smvm_load_inst(smvm *vm, smvm_inst *inst, long num) {
  for (size_t i = 0; i < num; i++) {
    smvm_inst c = inst[i];
    smvm_byte operand = c.code;

    // refer to [docs/BYTECODE.md]
    switch (c.mode) {
      case mode_indirect:
      case mode_register: {
        smvm_word short_inst = (operand << 6) | c.x << 3 | c.y;
        print_memory(&short_inst, 1);
        listmv_push(&vm->bytecode, &short_inst);
        break;
      }

      // TODO fix bit shifting (>> 32, << 32 is off)
      case mode_immediate: {
        smvm_size addr = vm->allocation.len;
        listmv_push(&vm->allocation, &c.y);
        break;
      }
      case mode_direct: {
        smvm_size inst = (smvm_size)operand << 56 | (smvm_size)c.x << 53 | c.y;
        break;
      }

      case mode_implicit: {
        listmv_push(&vm->bytecode, (smvm_word *)&c.code);
        break;
      }

      default:  // TODO error handler
        break;
    }
  }
}

void smvm_fetch_inst(smvm *vm) {}

void smvm_execute(smvm *vm) {
  smvm_word word;

  while (true) {
    // TODO: perhaps move this to another function?
    // TODO, all
  }
}

void smvm_free(smvm *vm) {}

// helper implementation

smvm_word smvm_next(smvm *vm) {
  smvm_word *next = (smvm_word *)listmv_at(&vm->bytecode, vm->reg[reg_ip]++);
  return *next;
}

/* util - implementation */

void listmv_init(listmv *ls, long size) {
  ls->len = 0;
  ls->cap = 8;
  ls->data = malloc(size * ls->cap * sizeof(char));
  ls->size = size;
}

void listmv_push(listmv *ls, void *data) {
  if (ls->len >= ls->cap) {
    ls->data = realloc(ls->data, (size_t)ls->cap * ls->size * sizeof(char) * 2);
    ls->cap *= 2;
  }

  memcpy(ls->data + ls->len, data, ls->size);
  ls->len++;
}

void *listmv_at(listmv *ls, smvm_size index) {
  return ls->data + index * ls->size;
}

void listmv_free(listmv *ls) {
  if (ls->data != NULL) free(ls->data);
}

#endif