#include <stdlib.h>

#include "smvm.h"

int main() {
  smvm vm;
  smvm_init(&vm);
  smvm_inst inst[4] = {
      (smvm_inst){op_print_int, mode_immediate, 0, 0, 69},
      (smvm_inst){op_print_int, mode_immediate, 0, 0, 420},
      (smvm_inst){op_print_int, mode_immediate, 0, 0, 0},
      (smvm_inst){op_halt, mode_implicit},
  };

  smvm_load_inst(&vm, inst, 4);
  smvm_execute(&vm);
  smvm_free(&vm);

  return 0;
}
