Casts in Hack convert values to different types. To assert a type
without changing its value, see [type assertions](type-assertions.md).

``` Hack
(float)1; // 1.0
(int)3.14; // 3, rounds towards zero

(bool)0; // false

(string)false; // ""
```

Casts are only supported for `bool`, `int`, `float` and `string`. See
[type conversions](../types/type-conversion.md) to understand what
value will be produced.
