# smvm - instructions
Instruction in smvm can be divided into many types:
1. [Memory management instructions](#memory-management-instructions)
    1. [mov](#1-mov-x-y)
    2. swap
2. [Arithmetic instructions](#arithmetic-instructions)
3. [Bitwise operation instructions](#bitwise-operation-instructions)
4. [Branching instructions](#branching-instructions)
5. [Misc. instructions](#misc-instructions)

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
These are pretty much selt explanatory.
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
