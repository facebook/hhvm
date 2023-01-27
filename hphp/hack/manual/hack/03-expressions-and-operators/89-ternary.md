The ternary operator `?` `:` is a shorthand for `if` statements. It is
an expression, so it evaluates to a value. For example:

```Hack no-extract
$days_in_feb = is_leap_year($year) ? 29 : 28;
```

It takes three operands `e1 ? e2 : e3`. If `e1` evaluates to a truthy
value, then the result is the evaluation of `e2`. Otherwise the result
is the evaluation of `e3`.

## Elvis Operator

There is also a two operand version `?:`, sometimes called the "elvis
operator". This results in the first operand if it evaluates to a truthy
value. For example:

``` Hack no-extract
$x = foo() ?: bar();

// Is equivalent to:
$tmp = foo();
$x = $tmp ? $tmp : bar();
```
