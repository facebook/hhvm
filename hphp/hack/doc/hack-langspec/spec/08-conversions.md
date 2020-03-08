# Conversions

## General

Some operators implicitly convert automatically the values of operands
from one type to another. Explicit conversion is performed using the
[cast operator](10-expressions.md#cast-operator).

If an expression is converted to its own type, the type and value of the
result are the same as the type and value of the expression.

When an expression of type `num` is converted, if that expression is currently 
an `int`, then `int` conversion rules apply; otherwise, `float` conversion rules apply.

When an expression of type `arraykey` is converted, if that expression is 
currently an `int`, then `int` conversion rules apply; otherwise, `string` 
conversion rules apply.

## Converting to Boolean Type

No non-`bool` type can be converted implicitly to `bool`. All other conversions 
must be explicit.

The [result type](http://docs.hhvm.com/manual/en/language.types.boolean.php#language.types.boolean.casting) is `bool`.

If the source type is `int` or `float`, then if the source value tests equal
to 0, the result value is `false`; otherwise, the result value is `true`.

If the source value is `null`, the result value is `false`.

If the source is an empty string or the string "0", the result value is
`false`; otherwise, the result value is `true`.

If the source is an array with zero elements, the result value is `false`;
otherwise, the result value is `true`.

If the source is an object, the result value is `true`.

If the source is a resource, the result value is `false`.

The library function [`boolval`](http://www.php.net/boolval) allows values to be converted to
`bool`.

## Converting to Integer Type

No non-`int` type can be converted implicitly to `int`. All other conversions must be explicit.

The [result type](http://docs.hhvm.com/manual/en/language.types.integer.php#language.types.integer.casting) is `int`.

If the source type is `bool`, then if the source value is `false`, the
result value is 0; otherwise, the result value is 1.

If the source type is `float`, for the values `INF`, `-INF`, and `NAN`, the
result value is implementation-defined. For all other values, if the
precision can be preserved, the fractional part is rounded towards zero
and the result is well defined; otherwise, the result is undefined.

If the source value is `null`, the result value is 0.

If the source is a [numeric string or leading-numeric string](05-types.md#the-string-type)
having integer format, if the precision can be preserved the result
value is that string's integer value; otherwise, the result is
undefined. If the source is a numeric string or leading-numeric string
having floating-point format, the string's floating-point value is
treated as described above for a conversion from `float`. The trailing
non-numeric characters in leading-numeric strings are ignored.  For any
other string, the result value is 0.

If the source is an array with zero elements, the result value is 0;
otherwise, the result value is 1.

If the source is a resource, the result is the resource's unique ID.

The library function [`intval`](http://docs.hhvm.com/manual/en/function.intval.php) allows values
to be converted to `int`.

## Converting to Floating-Point Type

No non-`float` type can be converted implicitly to `float`. All other 
conversions must be explicit.

The [result type](http://docs.hhvm.com/manual/en/language.types.float.php#language.types.float.casting) is `float`.

If the source type is `int`, if the precision can be preserved the result
value is the closest approximation to the source value; otherwise, the
result is undefined.

If the source is a [numeric string or leading-numeric string](05-types.md#the-string-type)
having integer format, the string's integer value is treated as
described above for a conversion from `int`. If the source is a numeric
string or leading-numeric string having floating-point format, the
result value is the closest approximation to the string's floating-point
value. The trailing non-numeric characters in leading-numeric strings
are ignored. For any other string, the result value is 0.

If the source is an array with zero elements, the result value is 0.0; 
otherwise, the result value is 1.0.

If the source is a resource, the result is the resource's unique ID.

The library function [`floatval`](http://www.php.net/floatval) allows values to be converted to
float.

## Converting to Number Type

The only implicit conversions to type `num` are from the types `int` and
`float`. There is no change in representation during such conversions. There are no explicit conversions.

The result type is `num`.

## Converting to String Type

Except for the [`classname` type](05-types.md#the-classname-type), no non-`string` type can be converted implicitly to `string`. All other conversions must be explicit.

The [result type](http://docs.hhvm.com/manual/en/language.types.string.php#language.types.string.casting) is `string`.

If the source type is `bool`, then if the source value is `false`, the
result value is the empty string; otherwise, the result value is "1".

If the source type is `int` or `float`, then the result value is a string
containing the textual representation of the source value (as specified
by the library function [`sprintf`](http://www.php.net/sprintf).

If the source value is `null`, the result value is an empty string.

If the source is an object, then if that object's class has a
[`__toString` method](16-classes.md#method-__tostring), the result value is the string returned
by that method; otherwise, the conversion is invalid.

If the source is a resource, the result value is an
implementation-defined string.

If the source type is the [`classname` type](05-types.md#the-classname-type), the result value is a string containing the corresponding fully qualified class or interface name without any leading `\`.

The library function [`strval`](http://www.php.net/strval) allows values to be converted to
`string`.

## Converting to Array Key Type

The only implicit conversions to type `arraykey` are from the types `int` 
and `string`. There is no change in representation during such conversions. 
There are no explicit conversions.

The result type is `arraykey`.

## Converting to Array Type

For arrays of different types, no implicit conversions exist. There are no 
explicit conversions.

## Converting to Object Type

An object type can be converted implicitly to any object type from which the first object type is derived directly or indirectly. There are no other implicit or explicit conversions.

## Converting to Interface Type

An object type can be converted implicitly to any interface type that object 
type implements directly or indirectly.

An interface type can be converted implicitly to any interface type from which 
the first interface type is derived directly or indirectly.

There are no other implicit or explicit conversions.

## Converting to Resource Type

The [predefined resource-like constants](05-types.md#resource-types)) `STDIN`, `STDOUT`, and 
`STDERR`, can be converted implicitly to resource. No other non-resource type 
can be so converted. No explicit conversions exist.

## Converting to Mixed Type

Any type can be converted implicitly to [`mixed`](05-types.md#nullable-types). No explicit conversions
exist.
