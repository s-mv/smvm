#include "smvm.h"

#include "asmv.h"
#include "dsmv.h"

instruction_info instruction_table[instruction_table_len] = {
    [op_halt] = {"halt", 4, 0, trap_fn}, [op_mov] = {"mov", 3, 2, mov_fn},
    [op_movu] = {"movu", 4, 2, movu_fn}, [op_movf] = {"movf", 4, 2, movf_fn},
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
    [op_scall] = {"scall", 5, 1, scall}, [op_getu] = {"getu", 4, 1, getu_fn},
    [op_puti] = {"puti", 4, 1, puti_fn}, [op_putu] = {"putu", 4, 1, putu_fn},
    [op_putf] = {"putf", 4, 1, putf_fn}, [op_puts] = {"puts", 4, 1, puts_fn}};

void smvm_init(smvm *vm) {
  *vm = (smvm){0};
  listmv_init(&vm->bytecode, sizeof(u8));
  listmv_init(&vm->memory, sizeof(u8));
  listmv_init(&vm->stack, sizeof(u8));
  listmv_init(&vm->syscalls, sizeof(smvm_syscall));
  vm->little_endian = is_little_endian();
}

void smvm_register_syscall(smvm *vm, void *fn, char *fn_name) {
  int id = vm->syscalls.len;
  smvm_syscall call = {.id = id, .function = fn, .name = fn_name};
  listmv_push(&vm->syscalls, &call);
}

void smvm_assemble(smvm *vm, char *code) {
  asmv assembler;
  asmv_init(&assembler);
  assembler.code = code;
  asmv_assemble(&assembler);
  if (vm->bytecode.data != NULL) listmv_free(&vm->bytecode);
  vm->bytecode = assembler.output.bytecode;  // ownership to vm
  vm->header = assembler.output.header;
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
      u8 code_width = num_ops != 3 ? 3 : 4;
      i8 offsets[3] = {0, 0, 0};

      vm->cache.widths[0] = (inst[1] >> 2) & 3;
      vm->cache.widths[1] = inst[1] & 3;
      vm->cache.widths[2] = (inst[3] >> 3) & 3;

      for (int i = 0; i < num_ops; i++) {
        u8 byte = (i == 2) ? 3 : 1;
        u8 bit_shift = (i == 2) ? 5 : (5 - i);
        vm->cache.widths[i] = 1 << vm->cache.widths[i];
        if ((inst[byte] >> bit_shift) & 1) offsets[i] = inst[code_width++];

        switch (inst[i] >> 6) {
          case mode_register:
            vm->cache.pointers[i] = &vm->registers[reg[i]];
            break;
          case mode_indirect:
            listmv_grow(&vm->memory, vm->registers[reg[i]] +
                                         vm->cache.widths[i] + offsets[i] + 1);
            vm->cache.pointers[i] =
                listmv_at(&vm->memory, vm->registers[reg[i]]);
            if (offsets[i] != 0) {
              u8 *ptr = (u8 *)vm->cache.pointers[i];
              ptr -= offsets[i];
            }
            break;
          case mode_direct: {
            u8 size = 1 << (reg[i] & 3);
            u64 address = 0;
            // reg also holds size
            mov_mem((u8 *)&address, inst + code_width, size);
            listmv_grow(&vm->memory, address + vm->cache.widths[i] + 1);
            code_width += size;
            vm->cache.pointers[i] = listmv_at(&vm->memory, address);
            break;
          }
          case mode_immediate:
            u8 size = 1 << (reg[i] & 3);
            vm->cache.data[i] = 0;
            if (vm->little_endian)
              mov_mem_reverse((u8 *)(&vm->cache.data[i]), inst + code_width,
                              size);
            else mov_mem((u8 *)(&vm->cache.data[i]), inst + code_width, size);
            vm->cache.data[i] += offsets[i];
            code_width += size;
            vm->cache.pointers[i] = &vm->cache.data[i];
            break;
          default:  // idk what this is for
            break;
        }
      }
      vm->cache.offset = code_width;
      instruction_table[code].fn(vm);
    }

    if (smvm_get_flag(vm, flag_t)) break;  // TODO, so much
  }
}

void smvm_free(smvm *vm) {
  listmv_free(&vm->memory);
  listmv_free(&vm->bytecode);
  listmv_free(&vm->stack);
  listmv_free(&vm->syscalls);
}

void smvm_disassemble(smvm *vm, char *code) {
  dsmv ds;
  dsmv_init(&ds);
  ds.bytecode = vm->bytecode;
  dsmv_disassemble(&ds);
  // code = malloc(ds.code.len);
  strncpy(code, (char *)ds.code.data, ds.code.len);
  dsmv_free(&ds);
}

void update_stack_pointer(smvm *vm) { vm->registers[reg_sp] = vm->stack.len; }

smvm_data_width min_space_neededu(u64 data) {
  if ((data >> 8) == 0) return smvm_reg8;
  if ((data >> 16) == 0) return smvm_reg16;
  if ((data >> 32) == 0) return smvm_reg32;
  return smvm_reg64;
}

smvm_data_width min_space_needed(i64 data) {
  if ((data >> 7) == 0 || (data >> 7) == -1) return smvm_reg8;
  if ((data >> 15) == 0 || (data >> 15) == -1) return smvm_reg16;
  if ((data >> 31) == 0 || (data >> 31) == -1) return smvm_reg32;
  return smvm_reg64;
}

void smvm_ip_inc(smvm *vm, u64 inc) { vm->registers[reg_ip] += inc; }

u8 *smvm_fetch_inst_addr(smvm *vm) {
  return (u8 *)listmv_at(&vm->bytecode, vm->registers[reg_ip]);
}

u16 smvm_get_flag(smvm *vm, smvm_flag flag) { return vm->flags & flag; }

void smvm_set_flag(smvm *vm, smvm_flag flag) { vm->flags |= flag; }

void smvm_reset_flag(smvm *vm, smvm_flag flag) { vm->flags &= ~flag; }

/* vm - opcode functions - implementation */

void trap_fn(smvm *vm) { smvm_set_flag(vm, flag_t); }
void mov_fn(smvm *vm) {
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
          vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void movu_fn(smvm *vm) {
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
          vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void movf_fn(smvm *vm) {
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
          vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void swap_fn(smvm *vm) {
  u64 temp;
  u8 width = vm->cache.widths[0] > vm->cache.widths[1] ? vm->cache.widths[0]
                                                       : vm->cache.widths[1];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)temp, width);
  mov_mem((u8 *)temp, (u8 *)vm->cache.pointers[1], width);
}
void lea_fn(smvm *vm) {}
void add_fn(smvm *vm) {
  i64 result = *vm->cache.pointers[1] + *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void addu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->cache.pointers[1] + *(u64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void addf_fn(smvm *vm) {
  f64 result = *(f64 *)vm->cache.pointers[1] + *(f64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void sub_fn(smvm *vm) {
  i64 result = *vm->cache.pointers[1] - *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void subu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->cache.pointers[1] - *(u64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void subf_fn(smvm *vm) {
  f64 result = *(f64 *)vm->cache.pointers[1] - *(f64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void mul_fn(smvm *vm) {
  i64 result = *vm->cache.pointers[1] * *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void mulu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->cache.pointers[1] * *(u64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void mulf_fn(smvm *vm) {
  f64 result = *(f64 *)vm->cache.pointers[1] * *(f64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void div_fn(smvm *vm) {
  i64 result = *vm->cache.pointers[1] / *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void divu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->cache.pointers[1] / *(u64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void divf_fn(smvm *vm) {
  f64 result = *(f64 *)vm->cache.pointers[1] + *(f64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void inc_fn(smvm *vm) {
  u64 op = *vm->cache.pointers[0] + 1;
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&op, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void dec_fn(smvm *vm) {
  u64 op = *vm->cache.pointers[0] - 1;
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&op, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void and_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] & *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void or_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] | *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void xor_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] ^ *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void shl_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] << *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void shr_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] >> *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void shli_fn(smvm *vm) {}
void shri_fn(smvm *vm) {}
void slc_fn(smvm *vm) {}
void src_fn(smvm *vm) {}
void jmp_fn(smvm *vm) {
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->cache.pointers[0],
          vm->cache.widths[0]);
}
void je_fn(smvm *vm) {
  if (*vm->cache.pointers[0] != *vm->cache.pointers[1]) {
    smvm_ip_inc(vm, 1);
    return;
  }  // else
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->cache.pointers[2],
          vm->cache.widths[2]);
}
void jne_fn(smvm *vm) {
  i64 left = 0, right = 0;
  mov_mem((u8 *)&left, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  mov_mem((u8 *)&right, (u8 *)vm->cache.pointers[1], vm->cache.widths[1]);
  if (left == right) {
    smvm_ip_inc(vm, vm->cache.offset);
    return;
  }  // else
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->cache.pointers[2],
          vm->cache.widths[2]);
}
void jl_fn(smvm *vm) {}
void loop_fn(smvm *vm) {
  stack_push(vm, (u8 *)&vm->registers[reg_ip], 8);
  vm->registers[reg_ip] = 0;
  smvm_ip_inc(vm, vm->cache.offset);
}
void call_fn(smvm *vm) {
  u64 addr = vm->registers[reg_ip] + vm->cache.offset;
  stack_push(vm, (u8 *)&addr, 8);
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->cache.pointers[0],
          vm->cache.widths[0]);
}
void ret_fn(smvm *vm) { vm->registers[reg_ip] = *(i64 *)stack_pop(vm, 8); }
void push_fn(smvm *vm) {
  stack_push(vm, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void pop_fn(smvm *vm) {
  u8 *data = stack_pop(vm, vm->cache.widths[0]);
  mov_mem((u8 *)vm->cache.pointers[0], data, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}

void scall(smvm *vm) {}

void getu_fn(smvm *vm) {
  u64 input;
  scanf("%lu", &input);
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&input, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void puti_fn(smvm *vm) {
  i64 data = parse_signed(*vm->cache.pointers[0], vm->cache.widths[0]);
  printf("%li\n", data);
  smvm_ip_inc(vm, vm->cache.offset);
}
void putu_fn(smvm *vm) {
  u64 data = 0;
  mov_mem((u8 *)&data, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  printf("%lu\n", data);
  smvm_ip_inc(vm, vm->cache.offset);
}
void putf_fn(smvm *vm) {
  f64 data = 0;
  mov_mem((u8 *)&data, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  printf("%lf\n", data);
  smvm_ip_inc(vm, vm->cache.offset);
}
void puts_fn(smvm *vm) {
  smvm_ip_inc(vm, vm->cache.offset);
  char *str = (char *)listmv_at(&vm->bytecode, vm->registers[reg_ip]);
  u64 len = 0;
  while (str[len] != '\0') len++;
  len++;
  printf("%s", str);
  fflush(stdout);
  smvm_ip_inc(vm, len);
}

/* vm - helpers - implementation */

void stack_push(smvm *vm, u8 *value, u64 width) {
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

bool is_little_endian() {
  uint32_t num = 1;
  return *(uint8_t *)&num == 1;
}
