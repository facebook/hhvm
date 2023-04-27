The binary operator `.` creates a string that is the concatenation of the left-hand operand and the right-hand operand, in that order. If
either or both operands have types other than `string`, their values are converted to type `string`. Consider the following example:

```Hack
"foo"."bar"; // "foobar"
```

Here are some more examples, which all involve conversion to `string`:

```Hack
-10 . NAN;               // string result with value "-10NAN"
INF . "2e+5";            // string result with value "INF2e+5"
true . null;             // string result with value "1"
```
