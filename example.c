#include <stdlib.h>

#include "smvm.h"

#define LEN (6)

int main() {
  smvm vm;
  smvm_init(&vm);

  smvm_inst inst[LEN] = {
      (smvm_inst){op_mov, mode_immediate, 0, reg_a, repr_f64(69.6f)},
      (smvm_inst){op_mov, mode_immediate, 0, reg_b, repr_f64(420.1f)},
      (smvm_inst){op_subf, mode_register, reg_a, reg_b, reg_a},
      (smvm_inst){op_print_float, mode_register, 0, 0, reg_a},
      (smvm_inst){op_print_intu, mode_immediate, 0, 0, -69},
      (smvm_inst){op_halt, mode_implicit},
  };

  smvm_load_inst(&vm, inst, LEN);
  // print_memory(vm.bytecode.data, vm.bytecode.len);
  smvm_execute(&vm);
  smvm_free(&vm);
  return 0;
}
