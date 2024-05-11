#include "smvm.h"

void print_uint16_binary(uint16_t value) {
  int i;
  for (i = 15; i >= 0; i--) {
    printf("%d", (value >> i) & 1);
    if (i % 4 == 0) printf(" ");
  }
  printf("\n");
}

int main() {
  smvm vm;
  smvm_init(&vm);
  smvm_inst inst[4] = {
      (smvm_inst){op_mov, mode_immediate, reg_a, 10},
      (smvm_inst){op_mov, mode_immediate, reg_b, 11},
      (smvm_inst){op_add, mode_register, reg_a, reg_b},
      (smvm_inst){op_halt, mode_implicit},
  };

  smvm_load_inst(&vm, inst, 4);

  for (int i = 0; i < vm.bytecode.len; i++)
    print_uint16_binary((smvm_word)vm.bytecode.data[i]);

  printf("\n");

  return 0;
}
