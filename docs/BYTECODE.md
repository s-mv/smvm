# Bytecode representation.

Something to make sure before anything is that this bytecode is
**independent of endianness**. What I mean is that bytecode represented
here will be stored as serial bits from left to right, paying no attention
to the device's endianness. Thus printing it traditionally could result in
very different outputs than expected.

## Layout of the bytecode
Operands and data in the bytecode **do not** have a fixed size in the SMVM.
However the bytecode has a _direction_ of data, as shown below:  
`opcode -> mode bits and width bits -> register bits -> data`

1. The opcode takes 6 bits.
2. The mode bits consistitute of 3, 3-bit groups. Each group corresponds to
one operand.
3. The width bits constitute of 3, 2-bit pairs. Each pair, again, is tied to
one operand.

These bits are followed by either register codes or address/data bits.
Registers are enumerated by 3 bits.

This is how bytecode for a large instruction typically ends up looking:
```
+--------+ +--------+ +--------+ +--------+ +--- ...
|22111111| |22334444| |44666555| |  223777| | data ...
+--------+ +--------+ +--------+ +--------+ +--- ...
```
1. opcode
2. (& 3.) mode bits (0xx = doesn't need data, 1xx = needs data)
4. width bits
5. register x (or in case of direct mode, data size)
6. register y
7. register z
Followed by data if any.
The last byte is only included for modes that require it.

For conevience here's the layout for bytecode for instructions with 3, 2, and
1 register-sized operand.

**3 operands:**
```
+--------+ +--------+ +--------+ +--------+
|22111111| |22333333| |22555444| |  222666|
+--------+ +--------+ +--------+ +--------+
```
**2 operands:**
```
+--------+ +--------+ +--------+
|22111111| |22  3333| |22555444|
+--------+ +--------+ +--------+
```
**1 operand:**
```
+--------+ +--------+ +--------+
|22111111| |2     33| |     444|
+--------+ +--------+ +--------+
```
As you can see, it's quite intuitive.

**Extra case:** Immediate mode opcodes are simply stored as a single address,
possibly followed by data.
```
+--------+ +--- ...
|  111111| | data ...
+--------+ +--- ...
```
