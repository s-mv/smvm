#include <stdio.h>

#include "asmv.h"
#include "smvm.h"

void trap_fn(smvm *vm) { smvm_set_flag(vm, flag_t); }
void mov_fn(smvm *vm) {
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
          vm->cache.widths[0]);
}
void movu_fn(smvm *vm) {
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
          vm->cache.widths[0]);
}
void movf_fn(smvm *vm) {
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)vm->cache.pointers[1],
          vm->cache.widths[0]);
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
}
void addu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->cache.pointers[1] + *(u64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void addf_fn(smvm *vm) {
  f64 result = *(f64 *)vm->cache.pointers[1] + *(f64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void sub_fn(smvm *vm) {
  i64 result = *vm->cache.pointers[1] - *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void subu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->cache.pointers[1] - *(u64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void subf_fn(smvm *vm) {
  f64 result = *(f64 *)vm->cache.pointers[1] - *(f64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void mul_fn(smvm *vm) {
  i64 result = *vm->cache.pointers[1] * *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void mulu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->cache.pointers[1] * *(u64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void mulf_fn(smvm *vm) {
  f64 result = *(f64 *)vm->cache.pointers[1] * *(f64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void div_fn(smvm *vm) {
  i64 result = *vm->cache.pointers[1] / *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void divu_fn(smvm *vm) {
  u64 result = *(u64 *)vm->cache.pointers[1] / *(u64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void divf_fn(smvm *vm) {
  f64 result = *(f64 *)vm->cache.pointers[1] + *(f64 *)vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void inc_fn(smvm *vm) {
  u64 op = *vm->cache.pointers[0] + 1;
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&op, vm->cache.widths[0]);
}
void dec_fn(smvm *vm) {
  u64 op = *vm->cache.pointers[0] - 1;
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&op, vm->cache.widths[0]);
}
void and_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] & *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void or_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] | *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void xor_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] ^ *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void shl_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] << *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void shr_fn(smvm *vm) {
  u64 result = *vm->cache.pointers[1] >> *vm->cache.pointers[2];
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&result, vm->cache.widths[0]);
}
void shli_fn(smvm *vm) {}
void shri_fn(smvm *vm) {}
void slc_fn(smvm *vm) {}
void src_fn(smvm *vm) {}
void jmp_fn(smvm *vm) {
  vm->registers[reg_bp] = 0;
  mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[2],
          vm->cache.widths[2]);
  vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
}
void je_fn(smvm *vm) {
  if (*vm->cache.pointers[0] != *vm->cache.pointers[1]) { return; }  // else
  vm->registers[reg_bp] = 0;
  mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[2],
          vm->cache.widths[2]);
  vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
}
void jne_fn(smvm *vm) {
  i64 left = 0, right = 0;
  mov_mem((u8 *)&left, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  mov_mem((u8 *)&right, (u8 *)vm->cache.pointers[1], vm->cache.widths[1]);
  if (left == right) { return; }  // else
  vm->registers[reg_bp] = 0;
  mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[2],
          vm->cache.widths[2]);
  vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
}
void jl_fn(smvm *vm) {
  i64 left = 0, right = 0;
  mov_mem((u8 *)&left, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  mov_mem((u8 *)&right, (u8 *)vm->cache.pointers[1], vm->cache.widths[1]);
  if (left < right) { return; }  // else
  vm->registers[reg_bp] = 0;
  mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[2],
          vm->cache.widths[2]);
  vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
}
void loop_fn(smvm *vm) { smvm_push(vm, (u8 *)&vm->registers[reg_ip], 8); }
void call_fn(smvm *vm) {
  u64 addr = vm->registers[reg_ip];
  smvm_push(vm, (u8 *)&addr, 8);
  vm->registers[reg_bp] = 0;
  mov_mem((u8 *)&vm->registers[reg_bp], (u8 *)vm->cache.pointers[0],
          vm->cache.widths[0]);
  vm->registers[reg_ip] = vm->cache.instruction->label_index - 1;
}
void ret_fn(smvm *vm) {
  vm->registers[reg_ip] = *(i64 *)smvm_pop(vm, 8);
  asmv_inst *inst =
      (asmv_inst *)listmv_at(&vm->instructions, vm->registers[reg_ip]);
  vm->registers[reg_bp] = inst->index;
}
void push_fn(smvm *vm) {
  smvm_push(vm, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
}
void pop_fn(smvm *vm) {
  u8 *data = smvm_pop(vm, vm->cache.widths[0]);
  mov_mem((u8 *)vm->cache.pointers[0], data, vm->cache.widths[0]);
}
void extern_fn(smvm *vm) {
  char *name = (char *)listmv_at(&vm->bytecode, vm->registers[reg_ip]);
  u64 len = strlen(name) + 1;

  for (u64 i = 0; i < vm->syscalls.len; i++) {
    smvm_syscall *syscall = (smvm_syscall *)listmv_at(&vm->syscalls, i);
    if (strcmp(syscall->name, name) == 0) { return; }
  }

  smvm_syscall syscall = {
      .id = vm->syscalls.len,
      .name = malloc(len * sizeof(char)),
      .function = NULL,
  };
  strcpy(syscall.name, name);

  listmv_push(&vm->syscalls, &syscall);
}
void scall_fn(smvm *vm) {
  char *name = (char *)listmv_at(&vm->bytecode, vm->registers[reg_ip]);
  u64 len = strlen(name) + 1;

  for (u64 i = 0; i < vm->syscalls.len; i++) {
    smvm_syscall *syscall = (smvm_syscall *)listmv_at(&vm->syscalls, i);
    if (strcmp(syscall->name, name) == 0) {
      ((void (*)(smvm *))syscall->function)(vm);
      return;
    }
  }

  fprintf(stderr, "Error: Syscall '%s' not found\n", name);
  smvm_set_flag(vm, flag_t);
}
void getu_fn(smvm *vm) {
  u64 input;
  scanf("%lu", &input);
  mov_mem((u8 *)vm->cache.pointers[0], (u8 *)&input, vm->cache.widths[0]);
}
void puti_fn(smvm *vm) {
  i64 data = parse_signed(*vm->cache.pointers[0], vm->cache.widths[0]);
  printf("%li\n", data);
}
void putu_fn(smvm *vm) {
  u64 data = 0;
  mov_mem((u8 *)&data, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  printf("%lu\n", data);
}
void putf_fn(smvm *vm) {
  f64 data = 0;
  mov_mem((u8 *)&data, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  printf("%lf\n", data);
}
void puts_fn(smvm *vm) {
  printf("%s", (char *)vm->cache.instruction->operands[0].data.str.data);
}
