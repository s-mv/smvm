# smvm - instructions
Instruction in smvm can be divided into many types:
- [smvm - instructions](#smvm---instructions)
  - [Memory management instructions](#memory-management-instructions)
    - [1. `mov x y`](#1-mov-x-y)
  - [Arithmetic instructions](#arithmetic-instructions)
    - [1. Addition instructions](#1-addition-instructions)
    - [2. Subtraction instructions](#2-subtraction-instructions)
    - [3. Multiplication instructions](#3-multiplication-instructions)
    - [4. Division instructions](#4-division-instructions)
    - [5. Increment/decrement operations](#5-incrementdecrement-operations)
  - [Bitwise operation instructions](#bitwise-operation-instructions)
    - [1. AND](#1-and)
    - [2. OR](#2-or)
    - [3. XOR](#3-xor)
  - [Branching instructions](#branching-instructions)
  - [Misc. instructions](#misc-instructions)

## Memory management instructions
### 1. `mov x y`
Moves data from y to x. Example usage:
```
mov   ra 32       # ra <- 32
mov   @100 32     # memory addr 100 <- 32
mov   @ra 32      # memory addr inside ra <- 32
mov   ra32 32     # first 32 bits of ra <- 32
mov   @100>16 32  # first 16 bits @100 <- 32
# ...
```

## Arithmetic instructions
These are pretty much self explanatory.
### 1. Addition instructions
```
add   x y z     # x = y + z
addu  x y z     # x = y + z (unsigned)
addf  x y z     # x = y + z (float)
addf  x y z32   # x = y + <some z, 32-bit> 
# ...
```
### 2. Subtraction instructions
```
add   x y z     # x = y - z
addu  x y z     # x = y - z (unsigned)
addf  x y z     # x = y - z (float)
addf  x y z32   # x = y - <some z, 32-bit> 
# ...
```
### 3. Multiplication instructions
```
add   x y z     # x = y * z
addu  x y z     # x = y * z (unsigned)
addf  x y z     # x = y * z (float)
addf  x y z32   # x = y * <some z, 32-bit> 
# ...
```
### 4. Division instructions
```
add   x y z     # x = y / z
addu  x y z     # x = y / z (unsigned)
addf  x y z     # x = y / z (float)
addf  x y z32   # x = y / <some z, 32-bit> 
# ...
```

### 5. Increment/decrement operations
Simply adds/subtracts 1 from the first operand.
```
inc ra          # ra = ra + 1
dec ra          # ra = ra - 1
# ...
```

## Bitwise operation instructions
### 1. AND
### 2. OR
### 3. XOR

## Branching instructions

## Misc. instructions
