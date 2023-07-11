In a few situations (documented in the following sections) the values of operands are implicitly converted from one type to another. Explicit
conversion is performed using the [cast operator](../expressions-and-operators/casting.md).

If an expression is converted to its own type, the type and value of the result are the same as the type and value of the expression.

When an expression of type `num` is converted, if that expression is currently an `int`, then `int` conversion rules apply; otherwise, `float`
conversion rules apply.

When an expression of type `arraykey` is converted, if that expression is currently an `int`, then `int` conversion rules apply; otherwise, `string`
conversion rules apply.

## Conversion to `bool`

No non-`bool` type can be converted implicitly to `bool`. All other conversions must be explicit.

If the source type is `int` or `float`, then if the source value tests equal to 0, the result value is `false`; otherwise, the result value is `true`.

If the source value is `null`, the result value is `false`.

If the source is an empty string or the string "0", the result value is `false`; otherwise, the result value is `true`.

If the source is an array with zero elements, the result value is `false`; otherwise, the result value is `true`.

If the source is an object, the result value is `true`, with some legacy exceptions:
- `SimpleXMLElement` can be false if there are no child elements
- The legacy Hack Collection classes `Vector`, `Map`, `Set` and their immutable variants can be false if there are no elements in the collection.

We strongly recommend not depending on these behaviors.

If the source is a resource, the result value is `false`.

The library function `boolval` allows values to be converted to `bool`.

## Conversion to `int`

No non-`int` type can be converted implicitly to `int`. All other conversions must be explicit.

If the source type is `bool`, then if the source value is `false`, the result value is 0; otherwise, the result value is 1.

If the source type is `float`, for the values `INF`, `-INF`, and `NAN`, the result value is implementation-defined. For all other values, if
the precision can be preserved, the fractional part is rounded towards zero and the result is well defined; otherwise, the result is undefined.

If the source value is `null`, the result value is 0.

If the source is a [numeric string or leading-numeric string](../built-in-types/string.md) having integer format, if the precision can be preserved the result
value is that string's integer value; otherwise, the result is undefined. If the source is a numeric string or leading-numeric string having
floating-point format, the string's floating-point value is treated as described above for a conversion from `float`. The trailing non-numeric
characters in leading-numeric strings are ignored.  For any other string, the result value is 0.

If the source is an array with zero elements, the result value is 0; otherwise, the result value is 1.

If the source is a resource, the result is the resource's unique ID.

The library function `intval` allows values to be converted to `int`.

## Converting to `float`

No non-`float` type can be converted implicitly to `float`. All other conversions must be explicit.

If the source type is `int`, if the precision can be preserved the result value is the closest approximation to the source value; otherwise, the
result is undefined.

If the source is a [numeric string or leading-numeric string](../built-in-types/string.md) having integer format, the string's integer value is treated as described
above for a conversion from `int`. If the source is a numeric string or leading-numeric string having floating-point format, the result value is
the closest approximation to the string's floating-point value. The trailing non-numeric characters in leading-numeric strings are ignored. For
any other string, the result value is 0.

If the source is an array with zero elements, the result value is 0.0; otherwise, the result value is 1.0.

If the source is a resource, the result is the resource's unique ID.

The library function `floatval` allows values to be converted to float.

## Converting to `num`

The only implicit conversions to type `num` are from the types `int` and `float`. There is no change in representation during such conversions. There
are no explicit conversions.

## Converting to `string`

Except for the type [`classname`](../built-in-types/classname.md), no non-`string` type can be converted implicitly to `string`. All other conversions must be explicit.

If the source type is `bool`, then if the source value is `false`, the result value is the empty string; otherwise, the result value is "1".

If the source type is `int` or `float`, then the result value is a string containing the textual representation of the source value (as specified by the
library function `sprintf`).

If the source value is `null`, the result value is an empty string.

If the source is an object, then if that object's class has a [`__toString` method](../classes/methods-with-predefined-semantics.md#method-__toString), the
result value is the
string returned by that method; otherwise, the conversion is invalid.

If the source is a resource, the result value is an implementation-defined string.

If the source type is the [`classname` type](../built-in-types/classname.md), the result value is a string containing the corresponding fully qualified class or
interface name without any leading `\`.

The library function `strval` allows values to be converted to `string`.

## Converting to `arraykey`

The only implicit conversions to type `arraykey` are from the types `int` and `string`. There is no change in representation during such
conversions. There are no explicit conversions.

## Converting to an Object Type

An object type can be converted implicitly to any object type from which the first object type is derived directly or indirectly. There are no
other implicit or explicit conversions.

## Converting to an Interface Type

An object type can be converted implicitly to any interface type that object type implements directly or indirectly.

An interface type can be converted implicitly to any interface type from which the first interface type is derived directly or indirectly.

There are no other implicit or explicit conversions.

## Converting to Resource Type

Standard IO streams returned by [file stream functions](../built-in-types/resources.md) `HH\\stdin()`, `HH\\stdout()`, and `HH\\stderr()`, can be converted implicitly to resource.
No other non-resource type can be so converted. No explicit conversions exist.

## Converting to Mixed Type

Any type can be converted implicitly to `mixed`. No explicit conversions exist.
