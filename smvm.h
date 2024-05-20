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

/* util */

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
      if (j % 4 == 0) printf(" ");
    }
  }
  printf("\n");
}

/* vm */

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
} smvm;

typedef enum smvm_opcode {
  // opcode takes 6 bits
  op_halt = 0b00000,  // halts the program
  op_mov = 0b00001,   // mov x y
  op_add = 0b00010,   // add x y
  op_sub = 0b00011,   // sub x y
  op_jmp = 0b00100,   // jmp (to [d])
  op_je = 0b00101,    // if (a == b) jmp (to [d])
  op_jne = 0b00110,   // if (a != b) jmp (to [d])
  op_jl = 0b00111,    // if (a < b) jmp (to [d])
  op_loop = 0b01000,  // if (c > 0) dec c ; jmp (to [d])
  op_inc = 0b01010,
  op_dec = 0b01001,
  // NOTE: 1_____ > print instructions
  // these might be temporary instructio#ns
  op_print_int = 0b10000,
  op_print_float = 0b100001,
  op_print_str = 0b100010,  // prints till it encounters 0x0000
} smvm_opcode;

typedef enum smvm_register {
  reg_a = 0b000,
  reg_b = 0b001,
  reg_c = 0b010,
  reg_d = 0b011,
  reg_ip = 0b100,
} smvm_register;

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

/* vm - inline helpers */

static void smvm_ip_inc(smvm *vm) { vm->registers[reg_ip]++; }

static u8 *smvm_fetch_inst_addr(smvm *vm) {
  return listmv_at(&vm->bytecode, vm->registers[reg_ip]);
}

/* vm - implementation */

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
            (c.x << 7) | operand,
            (c.x << 6) | (c.y << 3) | c.z,
        };
        listmv_push_array(&vm->bytecode, &inst, 2);
        break;
      }

      case mode_immediate: {
        u8 long_inst[10] = {
            operand,
            // 3 bit c.x 2 bit c.y
            (c.x << 3) | c.y,
            // c.z is either a memory address or data
            // either way it is 64 bit
            (c.z >> 56) & 0xff,
            (c.z >> 48) & 0xff,
            (c.z >> 40) & 0xff,
            (c.z >> 32) & 0xff,
            (c.z >> 24) & 0xff,
            (c.z >> 16) & 0xff,
            (c.z >> 8) & 0xff,
            c.z & 0xff,
        };

        listmv_push(&vm->allocation, &c.y);
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

// TODO revamp
void smvm_execute(smvm *vm) {
  // TODO this
  // 0 = MSB
  while (true) {
    printf("ip -> %d : ", vm->registers[reg_ip]);
    u8 byte0 = *smvm_fetch_inst_addr(vm);
    u8 opcode = (byte0 >> 2) & 0b11111;  // higher 5 bits
    u8 mode = byte0 & 0b11;              // lower 2 bits

    switch (opcode) {
      case op_halt:
        return;  // TODO, so much (sighs)
        break;

      case op_mov: {
        switch (mode) {
          case mode_register:
            smvm_ip_inc(vm);
            break;  // TODO

          case mode_indirect:
            break;  // TODO

          case mode_immediate: {
            smvm_ip_inc(vm);

            u8 byte1 = *smvm_fetch_inst_addr(vm);
            u8 x = byte1 >> 5;                      // higher 3 bits
            u64 y = ((u8)(byte1 & 0x11111) << 48);  // lower 5 bits
            // unpack the next 6 bytes
            y |= *(u64 *)smvm_fetch_inst_addr(vm) >> 16;
            printf("%d", y);
            u64 data = *(u64 *)listmv_at(&vm->memory, y);

            // finally move the data
            vm->registers[x] = data;
          } break;  // TODO

          case mode_direct:
            break;  // TODO

          default:
            break;
        }
        break;
      }

      default:
        break;
    }
    vm->registers[reg_ip]++;
    if (vm->registers[reg_ip] > 19) break;
  }
}

void smvm_free(smvm *vm) {
  listmv_free(&vm->allocation);
  listmv_free(&vm->bytecode);
  listmv_free(&vm->memory);
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

void listmv_push_array(listmv *ls, void *data, size_t num) {
  size_t required_capacity = ls->len + num;
  if (required_capacity > ls->cap) {
    while (ls->cap < required_capacity) {
      ls->cap *= 2;
    }
    ls->data = realloc(ls->data, ls->cap * ls->size);
  }

  memcpy(ls->data + ls->len * ls->size, data, num * ls->size);
  ls->len += num;
}

void *listmv_at(listmv *ls, u64 index) { return ls->data + index * ls->size; }

void listmv_free(listmv *ls) {
  if (ls->data != NULL) free(ls->data);
}

#endif