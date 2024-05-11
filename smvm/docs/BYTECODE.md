Bytecode representation.

# Register/Indirect addressing mode:
```
+--+------+--+---+---+
|  |     0| 1|  2|  3|
+--+------+--+---+---+
```
Actual size: 14 bit
Padding: 2 bit
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
