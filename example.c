#include <stdlib.h>

#include "smvm.h"

#define len (6)

int main() {
  smvm vm;
  smvm_init(&vm);

  smvm_inst inst[len] = {
      (smvm_inst){op_mov, mode_immediate, 0, reg_a, 69},
      (smvm_inst){op_mov, mode_immediate, 0, reg_b, 420},
      (smvm_inst){op_sub, mode_register, reg_a, reg_a, reg_b},
      (smvm_inst){op_print_int, mode_register, 0, 0, reg_a},
      (smvm_inst){op_print_int, mode_immediate, 0, 0, 420},
      (smvm_inst){op_halt, mode_implicit},
  };

  smvm_load_inst(&vm, inst, len);
  smvm_execute(&vm);
  smvm_free(&vm);
  return 0;
}
