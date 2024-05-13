Bytecode representation.

Something to make sure before anything is that this bytecode is
**independent of endianness**. What I mean is that bytecode represented
here will be stored as serial bits from left to right, paying no attention
to the device's endianness. Thus printing it traditionally would result in
very different outputs than expected.

(The reason I have to write that is that I have just spent countless minutes trying to figure out what's wrong with my code, or me, mostly.)

# Register/Indirect addressing mode:
```
<--6--><2>     <3><3->
+-----+--+  +-+--+---+
|00000011|  |xx222333|
+-----+--+  +-+--+---+
```
0 - op code
1 - mode of operation (00 or 10)
2 - register
3 - register

# Immediate/Direct addressing mode:
```
<--6--><2>  <3-><-5-->  <---8---->  <---8---->
+-----+--+  +--+-----+  +--------+  +--------+
|00000011|  |22233333|  |33333333|  |33333333|
+-----+--+  +--+-----+  +--------+  +--------+

<---8---->  <---8---->  <---8---->  <---8---->
+--------+  +--------+  +--------+  +--------+
|33333333|  |33333333|  |33333333|  |33333333|
+--------+  +--------+  +--------+  +--------+
```
0. op code
1. mode of operation (10 or 11)
2. register
3. memory address or allocated address (53 bit)

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
