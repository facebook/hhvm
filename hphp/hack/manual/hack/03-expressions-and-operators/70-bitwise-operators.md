Hack provides a range of bitwise operators. These assume that their
operands are `int`.

## Bitwise AND

The operator `&` performs a bitwise AND on its two `int` operands and produces an `int`. For example:

```Hack
0b101111 & 0b101;                        // result is 0b101

$lcase_letter = 0x73;                   // lowercase letter 's'
$ucase_letter = $lcase_letter & ~0x20;  // clear the 6th bit to make uppercase letter 'S'
```

## Bitwise OR

The operator `|` performs a bitwise OR on its two `int` operands and produces an `int`. For example:

```Hack
0b101111 | 0b101;                      // result is 0b101111

$ucase_letter = 0x41;                 // uppercase letter 'A'
$lcase_letter = $ucase_letter | 0x20; // set the 6th bit to make lowercase 'a'
```

## Bitwise XOR

The operator `^` performs a bitwise XOR on its two `int` operands and produces an `int`. For example:

```Hack
0b101111 ^ 0b101;  // result is 0b101010
```

## Shifting

The operator `<<` performs a bitwise left shift. It takes two `int`
operands and produces an `int`.

`e1 << e2` shifts `e1` left by `e2` bits, zero extending the value.

```Hack
0b101 << 2;     // result is 0b10100
10 << 3;        // result is 80
```

The operator `>>` performs a bitwise right shift.

``` Hack
0b1011 >> 2;    // result is 0b10
100 >> 2;       // result is 25
```

Note that right shifts extend the sign bit:

```Hack
(1 << 63) >> 63; // result is -1
```

This is because `1 << 63` is 0x8000000000000000, or -9223372036854775808.

## Bitwise Negation

The operator `~` performs a bitwise negation on its `int` operand and produces an `int`. For example:

```Hack
$lLetter = 0x73;                 // lowercase letter 's'
$uLetter = $lLetter & ~0b100000; // clear the 6th bit to make uppercase letter 'S'
```
