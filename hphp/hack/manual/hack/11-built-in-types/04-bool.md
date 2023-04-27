The Boolean type `bool` can store two distinct values, which correspond to the Boolean values `true` and `false`, respectively.

Consider the following example:

```Hack
function is_leap_year(int $yy): bool {
  return ((($yy & 3) === 0) && (($yy % 100) !== 0)) || (($yy % 400) === 0);
}

<<__EntryPoint>>
function main(): void {
  $result = is_leap_year(1900);
  echo "1900 is ".(($result === true) ? "" : "not ")."a leap year\n";
}
```

When called, function `is_leap_year` takes one argument, of type `int`, and returns a value of type `bool`. (A year is a leap year if
it is a multiple of 4 but not a multiple of 100&mdash;for example, 1700, 1800, and 1900 were *not* leap years&mdash;or it's a multiple of 400.
Some redundant grouping parentheses have been added to aid readability.)

The equality operators `===` and `!==`, and the logical operators `&&` and `||`, all produce `bool` results, with the ultimate result of the
large `bool` expression being returned from the function.

As `is_leap_year` is explicitly typed to return a `bool`, the local variable `$result` is inferred as having type `bool`. (Unlike function
parameters such as `$yy`, or a function return, a local variable *cannot* have an explicit type.)

The value of `$result` is compared to the `bool` literal `true`, and the `bool` result is used as the controlling expression of the `?:` operator.
