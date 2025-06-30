#include "mini_catch2.h"
#include "smvm.h"
#include "util.h"

smvm bake_vm(const char* code) {
  smvm vm;
  smvm_init(&vm);
  smvm_assemble(&vm, (char*)code);
  return vm;
}

bool assert_register(smvm* vm, smvm_register reg, i64 expected) {
  return vm->registers[reg] == expected;
}

TEST_CASE(test_mov_register) {
  smvm vm = bake_vm("mov ra 42\nhalt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_a], 42);
  smvm_free(&vm);
}

TEST_CASE(test_simple_arithmetic) {
  smvm vm = bake_vm("mov ra 5\nmov rb 7\nadd rc ra rb\nhalt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_a], 5);
  ASSERT_EQUAL(vm.registers[reg_b], 7);
  ASSERT_EQUAL(vm.registers[reg_c], 12);
  smvm_free(&vm);
}

TEST_CASE(test_subtraction) {
  smvm vm = bake_vm("mov ra 15\nmov rb 7\nsub rc ra rb\nhalt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_c], 8);
  smvm_free(&vm);
}

TEST_CASE(test_multiplication) {
  smvm vm = bake_vm("mov ra 6\nmov rb 7\nmul rc ra rb\nhalt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_c], 42);
  smvm_free(&vm);
}

TEST_CASE(test_division) {
  smvm vm = bake_vm("mov ra 20\nmov rb 5\ndiv rc ra rb\nhalt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_c], 4);
  smvm_free(&vm);
}

TEST_CASE(test_push_pop) {
  smvm vm = bake_vm(
      "mov ra 123\n"
      "push ra\n"
      "mov ra 456\n"
      "pop rb\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_a], 456);
  ASSERT_EQUAL(vm.registers[reg_b], 123);
  smvm_free(&vm);
}

TEST_CASE(test_jmp) {
  smvm vm = bake_vm(
      "mov ra 1\n"
      "jmp .skip\n"
      "mov ra 2\n"
      ".skip\n"
      "mov rb 3\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_a], 1);
  ASSERT_EQUAL(vm.registers[reg_b], 3);
  smvm_free(&vm);
}

TEST_CASE(test_conditional_jump) {
  smvm vm = bake_vm(
      "mov ra 5\n"
      "mov rb 5\n"
      "je ra rb .equal\n"
      "mov rc 0\n"
      "jmp .end\n"
      ".equal\n"
      "mov rc 1\n"
      ".end\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_c], 1);
  smvm_free(&vm);
}

TEST_CASE(test_inequality_jump) {
  smvm vm = bake_vm(
      "mov ra 5\n"
      "mov rb 10\n"
      "jne ra rb .notequal\n"
      "mov rc 0\n"
      "jmp .end\n"
      ".notequal\n"
      "mov rc 1\n"
      ".end\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_c], 1);
  smvm_free(&vm);
}

TEST_CASE(test_call_ret) {
  smvm vm = bake_vm(
      "mov ra 0\n"
      "call .subroutine\n"
      "jmp .end\n"
      ".subroutine\n"
      "mov ra 42\n"
      "ret\n"
      ".end\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_a], 42);
  smvm_free(&vm);
}

TEST_CASE(test_bitwise_operations) {
  smvm vm = bake_vm(
      "mov ra 5\n"
      "mov rb 3\n"
      "and rc ra rb\n"
      "or rd ra rb\n"
      "xor ra ra rb\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_c], 1);
  ASSERT_EQUAL(vm.registers[reg_d], 7);
  ASSERT_EQUAL(vm.registers[reg_a], 6);
  smvm_free(&vm);
}

TEST_CASE(test_inc_dec) {
  smvm vm = bake_vm(
      "mov ra 10\n"
      "inc ra\n"
      "mov rb 20\n"
      "dec rb\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_a], 11);
  ASSERT_EQUAL(vm.registers[reg_b], 19);
  smvm_free(&vm);
}

TEST_CASE(test_shift_operations) {
  smvm vm = bake_vm(
      "mov ra 1\n"
      "mov rb 2\n"
      "shl rc ra rb\n"
      "mov rd 16\n"
      "shr ra rd rb\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_c], 4);
  ASSERT_EQUAL(vm.registers[reg_a], 4);
  smvm_free(&vm);
}

TEST_CASE(test_listmv_operations) {
  listmv list;
  listmv_init(&list, sizeof(int));

  for (int i = 0; i < 10; i++) { listmv_push(&list, &i); }
  ASSERT_EQUAL(list.len, 10);

  for (int i = 0; i < 10; i++) {
    int* value = (int*)listmv_at(&list, i);
    ASSERT_EQUAL(*value, i);
  }

  for (int i = 9; i >= 0; i--) {
    int* value = (int*)listmv_pop(&list);
    ASSERT_EQUAL(*value, i);
  }
  ASSERT_EQUAL(list.len, 0);

  listmv_free(&list);
}

TEST_CASE(test_fibonacci) {
  smvm vm = bake_vm(
      "mov ra 0\n"
      "mov rb 1\n"
      "mov rc 10\n"
      ".loop\n"
      "dec rc\n"
      "jne rc 0 .continue\n"
      "jmp .end\n"
      ".continue\n"
      "mov rd rb\n"
      "add rb ra rb\n"
      "mov ra rd\n"
      "jmp .loop\n"
      ".end\n"
      "halt");
  smvm_execute(&vm);
  ASSERT_EQUAL(vm.registers[reg_b], 55);
  smvm_free(&vm);
}

int main(int argc, char** argv) { return run_all_tests(); }
