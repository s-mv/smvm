Bytecode representation.

Something to make sure before anything is that this bytecode is
**independent of endianness**. What I mean is that bytecode represented
here will be stored as serial bits from left to right, paying no attention
to the device's endianness. Thus printing it traditionally would result in
very different outputs than expected.

(The reason I have to write that is that I have just spent countless minutes trying to figure out what's wrong with my code, or me, mostly.)

# Register/Indirect addressing mode:
```
+------+--+---+---+--+
|     0| 1|  2|  3|  |
+------+--+---+---+--+
```
0 - op code
1 - mode of operation (00 or 10)
2 - register
3 - register

In a 64 bit queue 4 such operations can be stored.


# Immediate/Direct addressing mode:
```
+------+--+---+---xxxxxx+
|     0| 1|  2|        3|
+------+--+---+---xxxxxx+   (x = 8 bit)
```
0. op code
1. mode of operation (10 or 11)
2. register
3. memory address or allocated address (53 bit)

# Implicit mode instructions:
```
+------+--+x+
|     0|  | |
+------+--+x+               (x = 8 bit)
```
It pains me to make implicit instructions 16-bit when they could've been 6 (i.e. 8 with padding) bit instructions.

0. op code

There is no need for mode here since the op code is enough.

# Endianness
