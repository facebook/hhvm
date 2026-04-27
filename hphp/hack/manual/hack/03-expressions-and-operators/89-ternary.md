# Ternary

The ternary operator `?` `:` is a shorthand for `if` statements. It is
an expression, so it evaluates to a value. For example:

```hack no-extract
$days_in_feb = is_leap_year($year) ? 29 : 28;
```

It takes three operands `e1 ? e2 : e3`. If the bool `e1` evaluates to true, then the result is the evaluation of `e2`. If `e1` evalutes to false, the result
is the evaluation of `e3`.

## Elvis Operator

There is also a two operand version `?:`, sometimes called the "elvis
operator". If the first operand is true, the whole expression evalutes to true. If the first operand is false, the whole expression evalutes to the second operand. For example:

``` Hack no-extract
$x = foo() ?: bar();

// Is equivalent to:
$tmp = foo();
$x = $tmp ? $tmp : bar();
```
