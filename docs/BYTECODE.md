Bytecode representation.

Something to make sure before anything is that this bytecode is
**independent of endianness**. What I mean is that bytecode represented
here will be stored as serial bits from left to right, paying no attention
to the device's endianness. Thus printing it traditionally would result in
very different outputs than expected.

(The reason I have to write that is that I have just spent countless minutes trying to figure out what's wrong with my code, or me, mostly.)

## Register bits - layout
TODO, make a separate file for this.

Registers are represented in 5 bits in such a way:
```
aaaaabb
```
Here `a` depicts the actual register, while `b` represents the bit width (00 for 64, 01 for 32, 10 for 16, 11 for 8 bit).

# Register/Indirect addressing mode:
```
+-+----+-+  +---+----+  +-+-+----+
|x0000011|  |22233333|  |x2244444|
+-+----+-+  +---+----+  +-+-+----+
```
0 - op code
1 - mode of operation (00 or 10)
2 - register or nothing
3 - register
4 - register

# Immediate/Direct addressing mode:
```
+-+----+-+  +---+----+  +--------+  +--------+
|x0000011|  |22233333|  |xxxx2244|  |55555555|
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

# Implicit mode instructions:
```
+-----+--+
|xxx00000|
+-----+--+
```

0. op code

There is no need for mode here since the op code is enough.

# Endianness
TODO, lots to say here. ~~Lots of pain.~~

