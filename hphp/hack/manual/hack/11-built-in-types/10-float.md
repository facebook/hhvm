The floating-point type, `float`, allows the use of real numbers. It supports at least the range and precision of IEEE 754 64-bit double-precision
representation, and includes the special values minus infinity, plus infinity, and Not-a-Number (NaN).  Using predefined constant names, those
values are written as `-INF`, `INF`, and `NAN`, respectively.

The library functions `is_finite`, `is_infinite`, and `is_nan` indicate if a given floating-point value is finite, infinite, or a NaN, respectively.

Consider the following example:

```Hack
function average_float(float $p1, float $p2): float {
  return ($p1 + $p2) / 2.0;
}

<<__EntryPoint>>
function main(): void {
  $val = 3e6;
  $result = average_float($val, 5.2E-2);
  echo "\$result is ".$result."\n";
}
```

When called, function `average_float` takes two arguments, of type `float`, and returns a value of type `float`.

The literals `2.0`, `3e6`, and `5.2E-2` have type `float`, so the local variable `$val` is inferred as having type `float`. (Unlike function parameters
such as `$p1` and `$p2`, or a function return, a local variable *cannot* have an explicit type.) Then when `$val` and `5.2E-2` are passed to
`average_float`, the compiler sees that two `float`s were passed and two `float`s were expected, so the call is well-formed.
