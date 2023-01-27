The integer type `int` is signed and uses twos-complement representation for negative values. At least 64 bits are used, so the range of values that
can be stored is at least [-9223372036854775808, 9223372036854775807].

Namespace HH\Lib\Math contains the following integer-related constants: `INT64_MAX`, `INT64_MIN`, `INT32_MAX`, `INT32_MIN`, `INT16_MAX`, `INT16_MIN`,
and `UINT32_MAX`.

Refer to your compiler's documentation to find the behavior when the largest `int` value is incremented, the smallest value is decremented, and the
unary minus is applied to the smallest value.

Consider the following example:

```Hack
function is_leap_year(int $yy): bool {
  return ((($yy & 3) === 0) && (($yy % 100) !== 0)) || (($yy % 400) === 0);
}

<<__EntryPoint>>
function main(): void {
  $year = 2001;
  $result = is_leap_year($year);
  echo "$year is ".(($result === true) ? "" : "not ")."a leap year\n";
}
```

When called, function `is_leap_year` takes one argument, of type `int`, and returns a value of type `bool`. (A year is a leap year if it is a
multiple of 4 but not a multiple of 100&mdash;for example, 1700, 1800, and 1900 were *not* leap years&mdash;or it's a multiple of 400. Some redundant
grouping parentheses have been added to aid readability.)

The bitwise AND operator, `&`, and the remainder operator, `%`, require operands of type `int`.

Like `3`, `0`, `100`, and `400`, `2001` is an `int` literal, so the local variable `$year` is inferred as having type `int`. (Unlike function
parameters such as `$yy`, or a function return, a local variable *cannot* have an explicit type.) Then when `$year` is passed to `is_leap_year`,
the compiler sees that an `int` was passed and an `int` was expected, so the call is well-formed.
