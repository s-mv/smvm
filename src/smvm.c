#include "smvm.h"

#include <stdio.h>
#include <string.h>

#include "asmv.h"
#include "dsmv.h"
#include "util.h"

instruction_info instruction_table[instruction_table_len] = {
    [op_halt] = {"halt", 4, 0},     [op_mov] = {"mov", 3, 2},
    [op_movu] = {"movu", 4, 2},     [op_movf] = {"movf", 4, 2},
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
    [op_src] = {"src", 3, 3},       [op_jmp] = {"jmp", 3, 1},
    [op_je] = {"je", 2, 3},         [op_jne] = {"jne", 3, 3},
    [op_jl] = {"jl", 2, 3},         [op_loop] = {"loop", 4, 2},
    [op_call] = {"call", 4, 1},     [op_ret] = {"ret", 3, 0},
    [op_push] = {"push", 4, 1},     [op_pop] = {"pop", 3, 1},
    [op_extern] = {"extern", 6, 1}, [op_scall] = {"scall", 5, 1},
    [op_getu] = {"getu", 4, 1},     [op_puti] = {"puti", 4, 1},
    [op_putu] = {"putu", 4, 1},     [op_putf] = {"putf", 4, 1},
    [op_puts] = {"puts", 4, 1}};

void smvm_init(smvm *vm) {
  *vm = (smvm){0};
  listmv_init(&vm->bytecode, sizeof(u8));
  listmv_init(&vm->memory, sizeof(u8));
  listmv_init(&vm->stack, sizeof(u8));
  vm->little_endian = is_little_endian();
}

u64 smvm_find_syscall_index(smvm *vm, const char *name) {
  for (u64 i = 0; i < vm->syscalls.len; i++) {
    smvm_syscall *syscall = (smvm_syscall *)listmv_at(&vm->syscalls, i);
    if (strcmp(syscall->name, name) == 0) { return i; }
  }
  return (u64)-1;
}

void smvm_assemble(smvm *vm, char *code) {
  asmv assembler;
  asmv_init(&assembler, vm);
  assembler.code = code;
  asmv_assemble(&assembler);
  if (vm->bytecode.data != NULL) listmv_free(&vm->bytecode);
  vm->instructions = assembler.instructions;
  vm->bytecode = assembler.bytecode;  // ownership to vm
  vm->header = assembler.header;
  listmv_free(&vm->syscalls);
  vm->syscalls = assembler.syscalls;
  asmv_free(&assembler);
}

// TODO error return type
void smvm_execute(smvm *vm) {
  static void *instruction_labels[] = {
      &&trap_label, &&mov_label,  &&movu_label, &&movf_label,   &&swap_label,
      &&lea_label,  &&add_label,  &&addu_label, &&addf_label,   &&sub_label,
      &&subu_label, &&subf_label, &&mul_label,  &&mulu_label,   &&mulf_label,
      &&div_label,  &&divu_label, &&divf_label, &&inc_label,    &&dec_label,
      &&and_label,  &&or_label,   &&xor_label,  &&shl_label,    &&shr_label,
      &&shli_label, &&shri_label, &&slc_label,  &&src_label,    &&jmp_label,
      &&je_label,   &&jne_label,  &&jl_label,   &&loop_label,   &&call_label,
      &&ret_label,  &&push_label, &&pop_label,  &&extern_label, &&scall_label,
      &&getu_label, &&puti_label, &&putu_label, &&putf_label,   &&puts_label,
  };

  for (vm->registers[reg_ip] = 0; vm->registers[reg_ip] < vm->instructions.len;
       vm->registers[reg_ip]++) {
    if (smvm_get_flag(vm, flag_t)) break;  // TODO, so much

    asmv_inst *instruction =
        (asmv_inst *)listmv_at(&vm->instructions, vm->registers[reg_ip]);
    vm->cache.instruction = instruction;

    u8 code = instruction->code;
    u8 num_ops = instruction_table[code].num_ops;
    u8 code_width = num_ops != 3 ? 3 : 4;

    for (int j = 0; j < num_ops; j++) {
      asmv_operand *op = &instruction->operands[j];
      switch (op->mode) {
        case mode_register: {
          vm->cache.pointers[j] = &vm->registers[op->data.reg];
          break;
        }
        case mode_indirect: {
          listmv_grow(&vm->memory,
                      vm->registers[op->data.reg] + (1 << op->width));
          vm->cache.pointers[j] =
              listmv_at(&vm->memory, vm->registers[op->data.reg]);
          break;
        }
        case mode_direct: {
          listmv_grow(&vm->memory, op->data.unum + (1 << op->width));
          vm->cache.pointers[j] = listmv_at(&vm->memory, op->data.unum);
          break;
        }
        case mode_immediate: {
          vm->cache.data[j] = op->data.unum;
          vm->cache.pointers[j] = &vm->cache.data[j];
          break;
        }
        default: {
          fprintf(stderr, "Unknown operand mode\n");
          smvm_set_flag(vm, flag_t);
          return;
        }
      }
      vm->cache.widths[j] = 1 << op->width;

      if (op->data.type == asmv_str_type) {
        vm->cache.pointers[j] = (i64 *)listmv_at(&op->data.str, 0);
      }
    }

    vm->cache.offset = code_width;

    u64 temp, result_u64;
    u8 width;
    i64 result_i64, left, right;
    f64 result_f64;
    u8 *data;
    char *name;

    goto *instruction_labels[code];

  trap_label:
    smvm_set_flag(vm, flag_t);
    continue;

  mov_label:
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
            vm->cache.widths[0]);
    continue;

  movu_label:
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
            vm->cache.widths[0]);
    continue;

  movf_label:
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
            vm->cache.widths[0]);
    continue;

  swap_label:
    width = vm->cache.widths[0] > vm->cache.widths[1] ? vm->cache.widths[0]
                                                      : vm->cache.widths[1];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)temp, width);
    mov_mem((u8 *)temp, (u8 *)vm->cache.pointers[1], width);
    continue;

  lea_label:
    continue;  // TODO

  add_label:
    result_i64 = *vm->cache.pointers[1] + *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_i64,
            vm->cache.widths[0]);
    continue;

  addu_label:
    result_u64 = *(u64 *)vm->cache.pointers[1] + *(u64 *)vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  addf_label:
    result_f64 = *(f64 *)vm->cache.pointers[1] + *(f64 *)vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_f64,
            vm->cache.widths[0]);
    continue;

  sub_label:
    result_i64 = *vm->cache.pointers[1] - *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_i64,
            vm->cache.widths[0]);
    continue;

  subu_label:
    result_u64 = *(u64 *)vm->cache.pointers[1] - *(u64 *)vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  subf_label:
    result_f64 = *(f64 *)vm->cache.pointers[1] - *(f64 *)vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_f64,
            vm->cache.widths[0]);
    continue;

  mul_label:
    result_i64 = *vm->cache.pointers[1] * *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_i64,
            vm->cache.widths[0]);
    continue;

  mulu_label:
    result_u64 = *(u64 *)vm->cache.pointers[1] * *(u64 *)vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  mulf_label:
    result_f64 = *(f64 *)vm->cache.pointers[1] * *(f64 *)vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_f64,
            vm->cache.widths[0]);
    continue;

  div_label:
    result_i64 = *vm->cache.pointers[1] / *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_i64,
            vm->cache.widths[0]);
    continue;

  divu_label:
    result_u64 = *(u64 *)vm->cache.pointers[1] / *(u64 *)vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  divf_label:
    result_f64 = *(f64 *)vm->cache.pointers[1] + *(f64 *)vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_f64,
            vm->cache.widths[0]);
    continue;

  inc_label:
    result_u64 = *vm->cache.pointers[0] + 1;
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  dec_label:
    result_u64 = *vm->cache.pointers[0] - 1;
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  and_label:
    result_u64 = *vm->cache.pointers[1] & *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  or_label:
    result_u64 = *vm->cache.pointers[1] | *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  xor_label:
    result_u64 = *vm->cache.pointers[1] ^ *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  shl_label:
    result_u64 = *vm->cache.pointers[1] << *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  shr_label:
    result_u64 = *vm->cache.pointers[1] >> *vm->cache.pointers[2];
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result_u64,
            vm->cache.widths[0]);
    continue;

  shli_label:
    continue;

  shri_label:
    continue;

  slc_label:
    continue;

  src_label:
    continue;

  jmp_label:
    vm->registers[reg_bp] = 0;
    mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[2],
            vm->cache.widths[2]);
    vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
    continue;

  je_label:
    if (*vm->cache.pointers[0] != *vm->cache.pointers[1]) continue;
    // else
    vm->registers[reg_bp] = 0;
    mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[2],
            vm->cache.widths[2]);
    vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
    continue;

  jne_label:
    left = 0, right = 0;
    mov_mem((u8 *)&left, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
    mov_mem((u8 *)&right, (u8 *)vm->cache.pointers[1], vm->cache.widths[1]);
    if (left == right) continue;
    // else
    vm->registers[reg_bp] = 0;
    mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[2],
            vm->cache.widths[2]);
    vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
    continue;

  jl_label:
    left = 0, right = 0;
    mov_mem((u8 *)&left, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
    mov_mem((u8 *)&right, (u8 *)vm->cache.pointers[1], vm->cache.widths[1]);
    if (left < right) continue;
    // else
    vm->registers[reg_bp] = 0;
    mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[2],
            vm->cache.widths[2]);
    vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
    continue;

  loop_label:
    smvm_push(vm, (u8 *)&vm->registers[reg_ip], 8);
    continue;

  call_label:
    result_u64 = vm->registers[reg_ip];  // address
    smvm_push(vm, (u8 *)&result_u64, 8);
    vm->registers[reg_bp] = 0;
    mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[0],
            vm->cache.widths[0]);
    vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
    continue;

  ret_label:
    vm->registers[reg_ip] = *(i64 *)smvm_pop(vm, 8);
    asmv_inst *inst =
        (asmv_inst *)listmv_at(&vm->instructions, vm->registers[reg_ip]);
    vm->registers[reg_bp] = inst->index;
    continue;

  push_label:
    smvm_push(vm, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
    continue;

  pop_label:
    data = smvm_pop(vm, vm->cache.widths[0]);
    mov_mem((u8 *)vm->cache.pointers[0], data, vm->cache.widths[0]);
    continue;

  extern_label:
    name = (char *)vm->cache.pointers[0];

    u64 index = smvm_find_syscall_index(vm, name);

    if (index == (u64)-1) {
      smvm_syscall syscall = {
          .id = vm->syscalls.len,
          .name = malloc((strlen(name) + 1) * sizeof(char)),
          .function = NULL,
      };
      strcpy(syscall.name, name);
      listmv_push(&vm->syscalls, &syscall);

      vm->registers[reg_a] = syscall.id;
    } else {
      vm->registers[reg_a] = index;
    }
    continue;

  scall_label:
    temp = *vm->cache.pointers[0];  // index

    if (temp < vm->syscalls.len) {
      smvm_syscall *syscall = (smvm_syscall *)listmv_at(&vm->syscalls, temp);
      if (syscall->function != NULL) {
        syscall->function(vm);
        return;
        continue;

        fprintf(stderr,
                "Error: Syscall '%s' (index %ld) has no implementation\n",
                syscall->name, temp);
      } else
        fprintf(stderr, "Error: Syscall index %ld is out of bounds\n", temp);

      smvm_set_flag(vm, flag_t);
      continue;
    }
    continue;

  getu_label:
    scanf("%lu", &temp);
    mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&temp, vm->cache.widths[0]);
    continue;

  puti_label:
    result_i64 = parse_signed(*vm->cache.pointers[0], vm->cache.widths[0]);
    printf("%li\n", result_i64);
    fflush(stdout);
    continue;

  putu_label:
    result_u64 = 0;  // data
    mov_mem((u8 *)&result_u64, (u8 *)vm->cache.pointers[0],
            vm->cache.widths[0]);
    printf("%lu\n", result_u64);
    fflush(stdout);
    continue;

  putf_label:
    result_f64 = 0;
    mov_mem((u8 *)&result_f64, (u8 *)vm->cache.pointers[0],
            vm->cache.widths[0]);
    printf("%lf\n", result_f64);
    fflush(stdout);
    continue;

  puts_label:
    printf("%s", (char *)vm->cache.instruction->operands[0].data.str.data);
    fflush(stdout);
    continue;
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

