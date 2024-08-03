# smvm - the smv virtual machine
~A register based bytecode virtual machine implementation by smv.

Currently the docs are in making (or being "transported" from past
projects) so for now there's only [BYTECODE](./docs/BYTECODE.md). Updates
are major TODO.

This VM is primarily meant to be embedded into C. Practically nobody should be
using it barebones without something like a JIT compiler built on top of it.
But there's nothing stopping you from doing that. (evil laughter?)

This is mostly a personal project, a header-only VM that can be embedded into
larger systems.

---

## Contents:
1. [Usage](#usage)
    1. [Basic Usage](#basic-usage)
    2. [C interoperability](#c-interoperability)
2. [Build and Install](#build-and-install)

---

Know more:
1. [Instruction set](./INSTRUCTIONSET.md)
2. [Bytecode](./BYTECODE.md)
3. List of examples: [examples/](/examples/)

---

## Usage
The VM is meant to be embedded into 

### Basic Usage
### C Interoperability
TODO, write more.

## Build and Install
Currently the VM is a single header [smvm.h](/smvm.h). You can include the
file and start using it, no setup required. I know that some people will be
paranoid which is why I'll make it less "cluttered"... in the foreseeable
future.

**TODO**: Modularize smvm, asmv, dsmv and smvm-debug. Modify the makefile so
that the user can choose to either have a header-only implementation or use it
as a traditional library.
