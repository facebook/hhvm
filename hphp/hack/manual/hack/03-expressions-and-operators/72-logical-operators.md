Hack provides the conventional boolean operations.

## Logical AND `&&`

The operator `&&` calculates the boolean AND operation of its two operands.

```Hack no-extract
if (youre_happy() && you_know_it()) {
  clap_your_hands();
}
```

If either operand does not have a boolean type, it is converted to a
boolean first. The result is always a boolean.

`&&` is short circuiting, so it stops evaluation on the first `false`
result.

```Hack no-extract
$x = one() && two() && three();
```

The function `three` will not be called if `one()` or `two()` evaluate to `false`.

## Logical OR `||`

The operator `||` calculates the boolean OR operation of its two operands.

```Hack no-extract
if ($weekday === 6 || $weekday === 7) {
  echo "It's a weekend";
}
```

If either operand does not have a boolean type, it is converted to a
boolean first. The result is always a boolean.

`||` is short circuiting, so it stops evaluation on the first `true`
result.

```Hack no-extract
$x = one() || two() || three();
```

The function `three` will not be called if `one()` or `two()` evaluate to `true`.


## Logical NOT `!`

The operator `!` calculate the boolean negation of its operand.

If the operand does not have a boolean type, it is converted to a
boolean first. The result is always a boolean.

```Hack no-extract
while (!is_connected()) {
  connect();
}
```

If the operand has type `num`, `!$v` is equivalent to
`$v === 0 || $v === 0.0`.
