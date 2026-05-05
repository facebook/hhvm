# Casting

Casts in Hack convert values to different types. To assert a type
without changing its value, see [type assertions](/hack/expressions-and-operators/type-assertions).

``` Hack
(float)1; // 1.0
(int)3.14; // 3, rounds towards zero
(string)false; // ""
```

Casts are only supported for `int`, `float` and `string`.

## Casting to `int`

If the source type is `bool`, then if the source value is `false`, the result value is 0; otherwise, the result value is 1.

If the source type is `float`, for the values `INF`, `-INF`, and `NAN`, the result value is implementation-defined. For all other values, if
the precision can be preserved, the fractional part is rounded towards zero and the result is well defined; otherwise, the result is undefined.

If the source value is `null`, the result value is 0.

If the source is a [numeric string or leading-numeric string](/hack/built-in-types/string) having integer format, if the precision can be preserved the result
value is that string's integer value; otherwise, the result is undefined. If the source is a numeric string or leading-numeric string having
floating-point format, the string's floating-point value is treated as described above for a conversion from `float`. The trailing non-numeric
characters in leading-numeric strings are ignored.  For any other string, the result value is 0.

If the source is an array with zero elements, the result value is 0; otherwise, the result value is 1.

If the source is a resource, the result is the resource's unique ID.

The library function `intval` <FbInternalOnly>(Wrapped as `PHP\intval` in WWW)</FbInternalOnly> allows values to be converted to `int`.

## Casting to `float`

If the source type is `int`, if the precision can be preserved the result value is the closest approximation to the source value; otherwise, the
result is undefined.

If the source is a [numeric string or leading-numeric string](/hack/built-in-types/string) having integer format, the string's integer value is treated as described
above for a conversion from `int`. If the source is a numeric string or leading-numeric string having floating-point format, the result value is
the closest approximation to the string's floating-point value. The trailing non-numeric characters in leading-numeric strings are ignored. For
any other string, the result value is 0.

If the source is an array with zero elements, the result value is 0.0; otherwise, the result value is 1.0.

If the source is a resource, the result is the resource's unique ID.

The library function `floatval` <FbInternalOnly>(Wrapped as `PHP\floatval` in WWW)</FbInternalOnly> allows values to be converted to float.

## Casting to `string`

If the source type is `bool`, then if the source value is `false`, the result value is the empty string; otherwise, the result value is "1".

If the source type is `int` or `float`, then the result value is a string containing the textual representation of the source value (as specified by the
library function `sprintf`).

If the source value is `null`, the result value is an empty string.

Objects cannot be cast to strings.

If the source is a resource, the result value is an implementation-defined string.

The library function `strval` <FbInternalOnly>(Wrapped as `PHP\strval` in WWW)</FbInternalOnly> allows values to be converted to `string`.
