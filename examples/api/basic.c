#include <smvm/smvm.h>

int main() {
  smvm vm;

  smvm_init(&vm);

  char *code =
      "puts \"hello, word! here's my lucky number: \""
      "mov ra 42\n"
      "puti ra\n"
      "halt";

  smvm_assemble(&vm, code);
  smvm_execute(&vm);
  smvm_free(&vm);

  return 0;
}
