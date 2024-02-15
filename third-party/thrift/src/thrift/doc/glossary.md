# Glossary

Terms that have a special meaning or interpretation in Thrift.

## Specification Requirement Levels

Key terms used consistently throughout this specification to indicate the level
of requirement:

- **must** / **must not** - required
- **should** / **should not** - strongly encouraged
- **may** - optional

For more details regarding the meaning of these terms, see [rfc2119](https://www.ietf.org/rfc/rfc2119.txt).

## Kinds of Types

Terms used to classify types.

- *Thrift type* - The type specified in the Thrift IDL, for example `i32`.
- *native type* - A language specific type. For example `int` and `java.lang.Integer` are native types in Java.
- *standard type* - A native type that has built-in support in a Thrift library. Usually a primitive type (for example `int32_t` or `uint64_t` in C++), a native container (for example, `dict` or `list` in Python), or a class defined by Thrift code-gen.
- *non-standard type* - A native type that does not have built-in support in the Thrift runtime. For example `folly::dynamic` in C++.
- *default type* - The standard type used by Thrift when no override is specified.
- *adapted type* - A native type that is convertible to/from an underlying type when used with a Thrift library.
- *underlying type* - The underlying type of an adapted type, is usually the standard type being adapted, except when multiple adapters are applied. In which case, underlying type is the adapted type of the previous adapter.

## Release States

See [annotations/thrift.thrift](https://github.com/facebook/fbthrift/tree/main/thrift/annotation/thrift.thrift)

- [Experimental](https://github.com/facebook/fbthrift/tree/main/thrift/annotation/thrift.thrift#Experimental)
- [Beta](https://github.com/facebook/fbthrift/tree/main/thrift/annotation/thrift.thrift#Beta)
- Released
- [Deprecated](https://github.com/facebook/fbthrift/tree/main/thrift/annotation/thrift.thrift#Deprecated)
- [Legacy](https://github.com/facebook/fbthrift/tree/main/thrift/annotation/thrift.thrift#Legacy)
