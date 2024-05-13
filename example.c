#include "smvm.h"

int main() {
  smvm vm;
  smvm_init(&vm);
  smvm_inst inst[1] = {
      // (smvm_inst){op_mov, mode_immediate, reg_a, 10},
      // (smvm_inst){op_mov, mode_immediate, reg_b, 11},
      (smvm_inst){op_add, mode_register, reg_a, reg_b},
      // (smvm_inst){op_halt, mode_implicit},
  };

  smvm_load_inst(&vm, inst, 1);

  printf("sizeof ir and ad inst -> id %d ar %d\n", sizeof(smvm_inst_id),
         sizeof(smvm_inst_ar));

  for (int i = 0; i < vm.bytecode.len; i++)
    print_memory(listmv_at(&vm.bytecode, i), 2);

  printf("\n");

  return 0;
}
