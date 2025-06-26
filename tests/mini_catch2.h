#ifndef smv_smvm_mini_catch2_h
#define smv_smvm_mini_catch2_h

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

typedef void (*test_function)(void);

typedef struct test_case {
  const char* name;
  test_function func;
} test_case;

#define MAX_TESTS 256
static test_case test_cases[MAX_TESTS];
static int test_count = 0;

static inline void register_test(const char* name, test_function func) {
  if (test_count < MAX_TESTS) {
    test_cases[test_count].name = name;
    test_cases[test_count].func = func;
    test_count++;
  }
}

#define TEST_CASE(name)                                            \
  static void name(void);                                          \
  __attribute__((constructor)) static void register_##name(void) { \
    register_test(#name, name);                                    \
  }                                                                \
  static void name(void)

#define REQUIRE(expr)                                                       \
  do {                                                                      \
    if (!(expr)) {                                                          \
      fprintf(stderr, "[FAILED] %s:%d: Requirement failed: %s\n", __FILE__, \
              __LINE__, #expr);                                             \
      exit(1);                                                              \
    }                                                                       \
  } while (0)

#define ASSERT_EQUAL(actual, expected)                                \
  do {                                                                \
    long long _actual = (long long)(actual);                          \
    long long _expected = (long long)(expected);                      \
    if (_actual != _expected) {                                       \
      fprintf(stderr, "[FAILED] %s:%d: Equality assertion failed:\n", \
              __FILE__, __LINE__);                                    \
      fprintf(stderr, "  Expected: %lld\n", _expected);               \
      fprintf(stderr, "  Actual:   %lld\n", _actual);                 \
      exit(1);                                                        \
    }                                                                 \
  } while (0)

static inline int run_all_tests(void) {
  int passed = 0;
  int failed = 0;

  for (int i = 0; i < test_count; ++i) {
    printf("[RUNNING] %s\n", test_cases[i].name);
    fflush(stdout);

    int pid = fork();
    if (pid == 0) {
      test_cases[i].func();
      exit(0);
    } else if (pid > 0) {
      int status;
      waitpid(pid, &status, 0);
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        printf("[PASSED]  %s\n", test_cases[i].name);
        passed++;
      } else {
        printf("[FAILED]  %s\n", test_cases[i].name);
        failed++;
      }
    } else {
      fprintf(stderr, "Fork failed for test %s\n", test_cases[i].name);
      failed++;
    }
  }

  printf("\n%d tests run. Passed: %d. Failed: %d.\n", test_count, passed,
         failed);
  return failed == 0 ? 0 : 1;
}

#endif
