# While

The general format of a `while` statement is

`while  (`  *expression* ` )`  *statement*

If the expression tests `true`, the *single* statement that follows is executed, and the process is repeated. If the expression tests `false`,
control transfers to the point immediately following the end of the `while` statement. The loop body (that is, the single statement) is executed
zero or more times. Consider the following:

```hack
$i = 1;
while ($i <= 10) {
  echo "$i\t".($i * $i)."\n"; // output a table of squares
  ++$i;
}
```

The execution of a `while` statement is impacted by a subordinate
[`break` or `continue`](/hack/statements/break-and-continue).

The controlling expression is often a combination of relational, equality, and logical expressions.  For example:

```hack no-extract
while (($i <= 10 && $j !== 0) || !getStatus()) {
  // ...
}
```

The controlling expression must have type `bool` or a type that can be implicitly converted to `bool`.  For example, in `while (1)` ...,
`while (123)` ..., and `while (-1.234e24)` ..., in each case, the value of the expression is non-zero, which is implicitly converted to `true`. Only
zero-values are converted to `false`.

The `do`/`while` statement behaves slightly differently than `while` in that the former executes the loop body *before* it tests the
controlling expression, whereas `while` executes it after.
