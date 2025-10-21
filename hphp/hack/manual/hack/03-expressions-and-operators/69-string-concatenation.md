The binary operator `.` creates a string that is the concatenation of the left-hand operand and the right-hand operand, in that order. If
either or both operands have type `int`, their values are converted to type `string`. Consider the following examples:

```Hack
"foo"."bar"; // "foobar"
"A" . 25;    // string result with value "A25"
```
