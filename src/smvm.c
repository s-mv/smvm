#include "smvm.h"

#include <stdio.h>
#include <string.h>

#include "asmv.h"
#include "dsmv.h"

instruction_info instruction_table[instruction_table_len] = {
    [op_halt] = {"halt", 4, 0, trap_fn},
    [op_mov] = {"mov", 3, 2, mov_fn},
    [op_movu] = {"movu", 4, 2, movu_fn},
    [op_movf] = {"movf", 4, 2, movf_fn},
    [op_swap] = {"swap", 4, 2, swap_fn},
    [op_lea] = {"lea", 3, 2, lea_fn},
    [op_addu] = {"addu", 4, 3, addu_fn},
    [op_addf] = {"addf", 4, 3, addf_fn},
    [op_add] = {"add", 3, 3, add_fn},
    [op_subu] = {"subu", 4, 3, subu_fn},
    [op_subf] = {"subf", 4, 3, subf_fn},
    [op_sub] = {"sub", 3, 3, sub_fn},
    [op_mulu] = {"mulu", 4, 3, mulu_fn},
    [op_mulf] = {"mulf", 4, 3, mulf_fn},
    [op_mul] = {"mul", 3, 3, mul_fn},
    [op_divu] = {"divu", 4, 3, divu_fn},
    [op_divf] = {"divf", 4, 3, divf_fn},
    [op_div] = {"div", 3, 3, div_fn},
    [op_inc] = {"inc", 3, 1, inc_fn},
    [op_dec] = {"dec", 3, 1, dec_fn},
    [op_and] = {"and", 3, 3, and_fn},
    [op_or] = {"or", 2, 3, or_fn},
    [op_xor] = {"xor", 3, 3, xor_fn},
    [op_shli] = {"shli", 4, 3, shli_fn},
    [op_shl] = {"shl", 3, 3, shl_fn},
    [op_shri] = {"shri", 4, 3, shri_fn},
    [op_shr] = {"shr", 3, 3, shr_fn},
    [op_slc] = {"slc", 3, 3, slc_fn},
    [op_src] = {"src", 3, 3, src_fn},
    [op_jmp] = {"jmp", 3, 1, jmp_fn},
    [op_je] = {"je", 2, 3, je_fn},
    [op_jne] = {"jne", 3, 3, jne_fn},
    [op_jl] = {"jl", 2, 3, jl_fn},
    [op_loop] = {"loop", 4, 2, loop_fn},
    [op_call] = {"call", 4, 1, call_fn},
    [op_ret] = {"ret", 3, 0, ret_fn},
    [op_push] = {"push", 4, 1, push_fn},
    [op_pop] = {"pop", 3, 1, pop_fn},
    [op_extern] = {"extern", 6, 1, extern_fn},
    [op_scall] = {"scall", 5, 1, scall_fn},
    [op_getu] = {"getu", 4, 1, getu_fn},
    [op_puti] = {"puti", 4, 1, puti_fn},
    [op_putu] = {"putu", 4, 1, putu_fn},
    [op_putf] = {"putf", 4, 1, putf_fn},
    [op_puts] = {"puts", 4, 1, puts_fn}};

void smvm_init(smvm *vm) {
  *vm = (smvm){0};
  listmv_init(&vm->bytecode, sizeof(u8));
  listmv_init(&vm->memory, sizeof(u8));
  listmv_init(&vm->stack, sizeof(u8));
  listmv_init(&vm->syscalls, sizeof(smvm_syscall));
  vm->little_endian = is_little_endian();
}

void smvm_link_call(smvm *vm, void (*fn)(smvm *), const char *name) {
  smvm_syscall syscall = {
      .id = vm->syscalls.len,
      .function = fn,
      .name = malloc((strlen(name) + 1) * sizeof(char)),
  };

  strncpy(syscall.name, name, strlen(name) + 1);

  listmv_push(&vm->syscalls, &syscall);
}

void smvm_assemble(smvm *vm, char *code) {
  asmv assembler;
  asmv_init(&assembler);
  assembler.code = code;
  asmv_assemble(&assembler);
  if (vm->bytecode.data != NULL) listmv_free(&vm->bytecode);
  vm->instructions = assembler.instructions;
  vm->bytecode = assembler.output.bytecode;  // ownership to vm
  vm->header = assembler.output.header;
  asmv_free(&assembler);
}

// TODO error return type
void smvm_execute(smvm *vm) {
  for (vm->registers[reg_ip] = 0; vm->registers[reg_ip] < vm->instructions.len;
       vm->registers[reg_ip]++) {
    asmv_inst *instruction =
        (asmv_inst *)listmv_at(&vm->instructions, vm->registers[reg_ip]);
    vm->cache.instruction = instruction;

    u8 code = instruction->code;
    u8 num_ops = instruction_table[code].num_ops;
    u8 code_width = num_ops != 3 ? 3 : 4;

    for (int j = 0; j < num_ops; j++) {
      asmv_operand *op = &instruction->operands[j];
      switch (op->mode) {
        case mode_register:
          vm->cache.pointers[j] = &vm->registers[op->data.reg];
          break;
        case mode_indirect:
          listmv_grow(&vm->memory, vm->registers[op->data.reg] + op->width);
          vm->cache.pointers[j] =
              listmv_at(&vm->memory, vm->registers[op->data.reg]);
          break;
        case mode_direct:
          listmv_grow(&vm->memory, op->data.unum + op->width);
          vm->cache.pointers[j] = listmv_at(&vm->memory, op->data.unum);
          break;
        case mode_immediate:
          vm->cache.data[j] = op->data.unum;
          vm->cache.pointers[j] = &vm->cache.data[j];
          break;
        default:
          fprintf(stderr, "Unknown operand mode\n");
          smvm_set_flag(vm, flag_t);
          return;
      }
      vm->cache.widths[j] = 1 << op->width;

      if (op->data.type == asmv_str_type) {
        vm->cache.pointers[j] = (i64 *)listmv_at(&op->data.str, 0);
      }
    }

    vm->cache.offset = code_width;
    instruction_table[code].fn(vm);

    if (smvm_get_flag(vm, flag_t)) break;  // TODO, so much
  }
}

void smvm_free(smvm *vm) {
  listmv_free(&vm->memory);
  for (int i = 0; i < vm->instructions.len; i++) {
    asmv_inst instruction = *(asmv_inst *)listmv_at(&vm->instructions, i);
    for (int j = 0; j < instruction_table[instruction.code].num_ops; j++) {
      asmv_operand *op = &instruction.operands[j];
      if (op->data.type == asmv_str_type) { listmv_free(&op->data.str); }
    }
  }
  listmv_free(&vm->instructions);
  listmv_free(&vm->bytecode);
  listmv_free(&vm->stack);
  listmv_free(&vm->syscalls);
}

void smvm_disassemble(smvm *vm, char *code) {
  dsmv ds;
  dsmv_init(&ds);
  ds.bytecode = vm->bytecode;
  dsmv_disassemble(&ds);
  code = malloc(ds.code.len);
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

void smvm_bytecode_inc(smvm *vm, u64 inc) { vm->registers[reg_bp] += inc; }

u8 *smvm_fetch_byte_addr(smvm *vm) {
  return (u8 *)listmv_at(&vm->bytecode, vm->registers[reg_bp]);
}

u16 smvm_get_flag(smvm *vm, smvm_flag flag) { return vm->flags & flag; }

void smvm_set_flag(smvm *vm, smvm_flag flag) { vm->flags |= flag; }

void smvm_reset_flag(smvm *vm, smvm_flag flag) { vm->flags &= ~flag; }

/* vm - helpers - implementation */

void smvm_push(smvm *vm, u8 *value, u64 width) {
  listmv_grow(&vm->stack, vm->stack.len + width);
  mov_mem((u8 *)vm->stack.data, value, width);
  vm->stack.len += width;
  update_stack_pointer(vm);
}

u8 *smvm_pop(smvm *vm, u64 width) {
  u8 *bytes = listmv_pop_array(&vm->stack, width);
  update_stack_pointer(vm);
  return bytes;
}

bool is_little_endian() {
  uint32_t num = 1;
  return *(uint8_t *)&num == 1;
}
