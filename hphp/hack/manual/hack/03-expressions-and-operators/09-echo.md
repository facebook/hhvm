This intrinsic function converts the value of an expression to `string` (if necessary) and writes the string to standard output.  For example:

```Hack
$v1 = 23;
echo '>>'.$v1."<<\n"; // outputs ">>23<<"

$v3 = "abc{$v1}xyz";
echo "$v3\n";
```

For a discussion of value substitution in strings, see [string literals](../source-code-fundamentals/literals.md#string-literals__double-quoted-string-literals).
For conversion to strings, see [type conversion](../types/type-conversion.md#converting-to-string).
