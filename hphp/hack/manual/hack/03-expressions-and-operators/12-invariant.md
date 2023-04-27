This intrinsic function behaves like a function with a `void` return type. It is intended to indicate a programmer error for a condition that
should never occur.  For example:

```Hack no-extract
invariant($obj is B, "Object must have type B");
invariant(!$p is null, "Value can't be null");
$max = 100;
invariant(!$p is null && $p <= $max, "\$p's value %d must be <= %d", $p, $max);
```

If the first argument value tests true, the program continues execution; otherwise, the library function
`invariant_violation` is called. That function does not return; instead, it either throws an
exception of type `\HH\InvariantException`, or calls the handler previously registered by the library function
`invariant_callback_register`.

The first argument is a boolean expression. The second argument is a string that can contain
text and/or optional formatting information as understood by the library function `Str\format`.  The optional
comma-separated list of values following the string must match the set of types expected by the optional formatting information inside that string.
