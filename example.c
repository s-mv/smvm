#include "smvm.h"

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

  return 0;
}
