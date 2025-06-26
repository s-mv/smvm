# smvm - the smv virtual machine
~A register based bytecode virtual machine implementation by smv.

Lightweight and fast, written under 1400 lines of code.

Currently the docs are in making (or being "transported" from past
projects) so for now there's only [BYTECODE](docs/BYTECODE.md) and
[INSTRUCTIONSET](docs/INSTRUCTIONSET.md). Updates are major TODO.

This VM is primarily meant to be embedded into C. Practically nobody should be
using it barebones without something like a JIT compiler built on top of it.
But there's nothing stopping you from doing that. (evil laughter?)

This is mostly a personal project, something that can be embedded into
larger systems.

---

## Contents:
- [smvm - the smv virtual machine](#smvm---the-smv-virtual-machine)
  - [Contents:](#contents)
  - [Usage](#usage)
    - [Basic Usage](#basic-usage)
    - [C Interoperability](#c-interoperability)
  - [Build and Install](#build-and-install)

---

Know more:
1. [Instruction set](./INSTRUCTIONSET.md)
2. [Bytecode](./BYTECODE.md)
3. List of examples: [examples/](/examples/)

---

## Usage
The VM is meant to be embedded into C but it can be used as a standalone, to
run assembly as well as smvm bytecode.

### Basic Usage

To use SMVM as a standalone VM:

1. Build the VM:
```bash
make dev
```

2. Run an assembly file:
```bash
make run_asmv FILE=examples/test.asmv
# or directly:
./out/smvm examples/test.asmv
```

3. Run tests:
```bash
make test
```

### C Interoperability

To embed SMVM in your C program:

1. Install the library:
```bash
make install
```

2. Include the headers in your C file:
```c
#include <smvm/smvm.h>
```

3. Basic usage example:
```c
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
```

4. Compile your program with:
```bash
cc your_program.c -lsmvm
```

## Build and Install

1. Build the development version:
```bash
make dev
```

2. Install the library system-wide:
```bash
make install
```

3. Build libraries only:
```bash
make out/libsmvm.a    # Static library
make out/libsmvm.so   # Shared library
```

For more build options:
```bash
make help
```
