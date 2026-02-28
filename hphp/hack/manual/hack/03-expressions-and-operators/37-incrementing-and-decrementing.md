# Incrementing / Decrementing

Hack provides `++` and `--` syntax for incrementing and decrementing
numbers.

The following are equivalent:

```hack no-extract
$i = $i + 1;
$i += 1;
$i++;
++$i;
```

Similarly for decrement:

```hack no-extract
$i = $i - 1;
$i -= 1;
$i--;
--$i;
```

This is typically used in for loops:

```hack
for ($i = 1; $i <= 10; $i++) {
  // ...
}
```

Note that `++` and `--` are statements, not expressions. They cannot
be used in larger expressions.

```hack error
$x = 0;
$y = $x++; // Parse error.
```

Instead, the above code must be written as statements.

```hack
$x = 0;
$y = $x + 1;
$x++;
```
