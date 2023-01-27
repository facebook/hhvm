Hack provides the standard arithmetic operators. These only operate on numeric types: `int` or `float`.

## Addition

The operator `+` produces the sum of its operands.

If both operands have type `int`, the result is `int`. Otherwise, the
operands are converted to `float` and the result is `float`.

```Hack
-10 + 100;       // int with value 90
100 + -3.4e2;    // float with value -240
9.5 + 23.444;    // float with value 32.944
```

## Subtraction

The operator `-` produces the difference of its operands.


If both operands have type `int`, the result is `int`. Otherwise, the
operands are converted to `float` and the result is `float`.

```Hack
-10 - 100;       // int with value -110
100 - -3.4e2;    // float with value 440
9.5 - 23.444;    // float with value -13.944
```

## Multiplication

The operator `*` produces the product of its operands.

If both operands have type `int`, the result is `int`. Otherwise, the
operands are converted to `float` and the result is `float`.

```Hack
-10 * 100;        // int result with value -1000
100 * -3.4e10;    // float result with value -3400000000000.0
```

## Division

The operator `/` produces the quotient from dividing the
left-hand operand by the right-hand one. Dividing by `0` will produce
an exception.

If both operands have type `int`, and the result can be represented
exactly as an `int`, then the result is an `int`. Otherwise, the result is `float`.

```Hack
300 / 100;       // int result with value 3
100 / 123;       // float result with value 0.8130081300813
12.34 / 2.3;     // float result with value 5.3652173913043
```

## Modulo

The operator `%` produces the `int` remainder from dividing the
left-hand `int` operand by the right-hand `int` operand. If the right
hand side is 0, an exception is thrown.

```Hack
5 % 2;     // int result with value 1
```

## Exponent

The operator `**` produces the result of raising the value of its
left-hand operand to the power of the right-hand one.


If both operands have non-negative integer values and the result can be represented as
an `int`, the result has type `int`; otherwise, the result has type `float`.

```Hack
2 ** 3;        // int with value 8
2 ** 3.0;      // float with value 8.0
2.0 ** 3.0;    // float with value 8.0
```

## Unary Plus

The unary plus operator `+` requires an `int` or `float` value, but
has no effect. It exists for symmetry.

The following are equivalent:

```Hack
$v = +10;
$v = 10;
```

## Unary Minus

The unary minus operator `-` requires an `int` or `float` value, and
returns the negated value.

```Hack
$v = 10;
$x = -$v; // $x has value -10
```

Note that due to underflow, negating the smallest negative value
produces the same value.
