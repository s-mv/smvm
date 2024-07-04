# Bytecode representation.

Something to make sure before anything is that this bytecode is
**independent of endianness**. What I mean is that bytecode represented
here will be stored as serial bits from left to right, paying no attention
to the device's endianness. Thus printing it traditionally would result in
very different outputs than expected.

(The reason I have to write that is that I have just spent countless minutes
trying to figure out what's wrong with my code, or me, mostly.)

## Layout of the bytecode
Operands and data in the bytecode **do not** have a fixed size in the SMVM.
However the bytecode has a _direction_ of data, as shown below:  
`opcode -> info bits -> width bits -> data`

1. The opcode takes 6 bits.
2. The info bits consistitute of 3, 2-bit pairs. Each pair corresponds to one
operand.
3. The width bits constitute of 3, 2-bit pairs. Each pair, again, is tied to
one operand.

These bits are followed by either register codes or address/data bits.
Registers are enumerated by 3 bits.

This is how bytecode typically ends up looking:
```
+--------+ +--------+ +--------+ +--------+ +--- ...
|22111111| |22333333| |22444555| |     666| | data ...
+--------+ +--------+ +--------+ +--------+ +--- ...
```
1. opcode
2. info bits
3. width bits
4. register x
5. register y
6. register z
Followed by data if any. The last byte isn't included for modes that require it.

**Exception**: Immediate mode opcodes are simply stored as a single address.
```
+--------+
|  111111|
+--------+
```

<!-- # Register
# Indirect addressing mode:
```
+-+----+-+  +---+----+  +-+-+----+
|00000011|  |12233333|  |22244444|
+-+----+-+  +---+----+  +-+-+----+
```
0 - op code
1 - mode of operation (00 or 10)
2 - register or nothing
3 - register
4 - register

**Example**: `add a b c` translates to `a = b + c`. Or `add a b [c]`
translates to `a = b + [c]`.

# Immediate/Direct addressing mode:
```
+-+----+-+  +---+----+  +--------+  +--------+
|00000011|  |12233333|  |xxx44222|  |55555555|
+-+----+-+  +---+----+  +--------+  +--------+

+--------+  +--------+  +--------+  +-------
|55555555|  |55555555|  |55555555|  |555555...
+--------+  +--------+  +--------+  +-------
```
0. op code
1. mode of operation (01 or 11)
2. register or nothing
3. register
4. width of memory
5. data (8/16/32/64 bit for int, 32/64 bit for float)
OR memory address (64 bit)

**Example**: `add a b 32` translates to `a = b + 32`. Or `add a b [32]`
translates to `a = b + [32]`.

# Implicit mode instructions:
```
+-----+--+
|xx000000|
+-----+--+
```

0. op code

There is no need for mode here since the op code is enough.

# The reverse mode bit.

The reverse mode bit only works for direct and indirect addressing modes.
The isolated bit of the mode bits is referred to as the reverse mode bit.
If the reverse bit is 1, when the instructios are in such a manner:
- `add [a] b c` as opposed to `add a b [c]`
- `add [32] b c`
- `mov [a] b` as opposed to `mov a [b]`
- `mov [32] b`
and so on.

The reason behind its naming is that the bytecode for `mov a [b]` and
`mov [b] a` is identical, the only difference being the reverse mode bit.

# Endianness
TODO, lots to say here. ~~Lots of pain.~~ -->
