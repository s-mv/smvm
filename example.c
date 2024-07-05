#include <stdio.h>
#include <stdlib.h>

#include "smvm.h"

// #include "smvm.h"

#define LEN (6)

char *readfile(const char *fname) {
  FILE *fp = fopen(fname, "rb");
  long file_size;

  if (fp == NULL) {
    perror("Error opening file");
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  rewind(fp);

  char *content = (char *)malloc(file_size + 1);

  if (content == NULL) {
    perror("Error allocating memory");
    fclose(fp);
    return NULL;
  }

  size_t bytes_read = fread(content, 1, file_size, fp);

  if (bytes_read != file_size) {
    perror("Error reading file");
    free(content);
    fclose(fp);
    return NULL;
  }

  content[file_size] = '\0';

  fclose(fp);
  return content;
}

int main() {
  smvm vm;
  char *content = readfile("example.asmv");

  smvm_init(&vm);
  printf("%s\n", content);

  smvm_assemble(&vm, content, false);

  free(content);
  smvm_free(&vm);
  return 0;
}

// int main() {
//   smvm vm;
//   smvm_init(&vm);

//   smvm_inst inst[LEN] = {
//       (smvm_inst){op_mov, mode_immediate, 0, reg_a, repr_f64(69.6f)},
//       (smvm_inst){op_mov, mode_immediate, 0, reg_b, repr_f64(420.1f)},
//       (smvm_inst){op_subf, mode_register, reg_a, reg_b, reg_a},
//       (smvm_inst){op_printf, mode_register, 0, 0, reg_a},
//       (smvm_inst){op_printu, mode_immediate, 0, 0, -69},
//       (smvm_inst){op_halt, mode_implicit},
//   };

//   smvm_load_inst(&vm, inst, LEN);
//   // print_memory(vm.bytecode.data, vm.bytecode.len);
//   smvm_execute(&vm);
//   smvm_free(&vm);
//   return 0;
// }
