The `for` statement is typically used to step through a range of values in ascending or descending increments, performing some set of operations
on each value.  For example:

```Hack
for ($i = 1; $i <= 5; ++$i) {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
}

$i = 1;
for (; $i <= 5; ) {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
}

$i = 1;
for (; ; ) {
  if ($i > 5)
    break;
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
}
```

In the first `for` loop above, let's call `$i = 1` the *for-initializer*, `$i <= 10` the *for-control*, and `++$i` the *for-end-of-loop-action*.
Each of these three parts can contain a comma-separated list of expressions.  For example:

```Hack no-extract
for ($i = 1, $j = 10; f($i), $i <= 10; $i = $i + 2, --$j) {
  // ...
}
```

The group of expressions in *for-initializer* is evaluated once, left-to-right, for their side-effects only. Then the group of expressions
in *for-control* is evaluated left-to-right (with all but the right-most one for their side-effects only), with the right-most expression's
value being tested. If that tests `true`, the loop body is executed, and the group of expressions in *for-end-of-loop-action* is evaluated
left-to-right, for their side-effects only. Then the process is repeated starting with *for-control*. If the right-most expression in
*for-control* tests `false`, control transfers to the point immediately following the end of the for statement. The loop body is executed zero or more times.

The controlling expression&mdash;the right-most expression in *for-control*---must have type `bool` or be implicitly convertible to that type.

Any or all of the three parts of the first line of a for statement can be omitted, as shown. If *for-initializer* is omitted, no action
is taken at the start of the loop processing. If *for-control* is omitted, this is treated as if *for-control* was an expression with the
value `true`. If *for-end-of-loop-action* is omitted, no action is taken at the end of each iteration.
