Bytecode representation.

Something to make sure before anything is that this bytecode is
**independent of endianness**. What I mean is that bytecode represented
here will be stored as serial bits from left to right, paying no attention
to the device's endianness. Thus printing it traditionally would result in
very different outputs than expected.

(The reason I have to write that is that I have just spent countless minutes trying to figure out what's wrong with my code, or me, mostly.)

# Register/Indirect addressing mode:
```
++----+--+  +-+--+---+
|21100000|  |22333444|
++----+--+  +-+--+---+
```
0 - op code
1 - mode of operation (00 or 10)
2 - register or nothing
3 - register
4 - register

# Immediate/Direct addressing mode:
```
++----+--+  +-+--+---+  +--------+  +--------+
|x0000011|  |xx222333|  |44444444|  |44444444|
++----+--+  +-+--+---+  +--------+  +--------+

+--------+  +--------+  +--------+  +--------+
|44444444|  |44444444|  |44444444|  |44444444|
+--------+  +--------+  +--------+  +--------+

+--------+  +--------+
|44444444|  |44444444|
+--------+  +--------+
```
0. op code
1. mode of operation (10 or 11)
2. register or none
3. register
4. memory address or allocated address (64 bit)

# Implicit mode instructions:
```
+-----+--+
|000000xx|
+-----+--+
```

0. op code

There is no need for mode here since the op code is enough.

# Endianness
TODO, lots to say here.
