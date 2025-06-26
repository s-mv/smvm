#include "smvm.h"

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
  smvm_push(vm, (u8 *)&vm->registers[reg_ip], 8);
  vm->registers[reg_ip] = 0;
  smvm_ip_inc(vm, vm->cache.offset);
}
void call_fn(smvm *vm) {
  u64 addr = vm->registers[reg_ip] + vm->cache.offset;
  smvm_push(vm, (u8 *)&addr, 8);
  vm->registers[reg_ip] = 0;
  mov_mem((u8 *)&vm->registers[reg_ip], (u8 *)vm->cache.pointers[0],
          vm->cache.widths[0]);
}
void ret_fn(smvm *vm) { vm->registers[reg_ip] = *(i64 *)smvm_pop(vm, 8); }
void push_fn(smvm *vm) {
  smvm_push(vm, (u8 *)vm->cache.pointers[0], vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void pop_fn(smvm *vm) {
  u8 *data = smvm_pop(vm, vm->cache.widths[0]);
  mov_mem((u8 *)vm->cache.pointers[0], data, vm->cache.widths[0]);
  smvm_ip_inc(vm, vm->cache.offset);
}
void extern_fn(smvm *vm) {
  smvm_ip_inc(vm, vm->cache.offset);
  char *name = (char *)listmv_at(&vm->bytecode, vm->registers[reg_ip]);
  u64 len = strlen(name) + 1;

  smvm_ip_inc(vm, len);
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
  smvm_ip_inc(vm, vm->cache.offset);
  char *name = (char *)listmv_at(&vm->bytecode, vm->registers[reg_ip]);
  u64 len = strlen(name) + 1;

  for (u64 i = 0; i < vm->syscalls.len; i++) {
    smvm_syscall *syscall = (smvm_syscall *)listmv_at(&vm->syscalls, i);
    if (strcmp(syscall->name, name) == 0) {
      smvm_ip_inc(vm, len);
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
