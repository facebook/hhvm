---
state: beta
---

# Data Types

Types that can be (de)serialized using [data protocols](../protocol/data.md).

See also [schema.thrift](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift).

## Categories

Shorthand names used for specific groups of types:

Category | Types
---|---
[*integer*](#primitive-types) | `bool`, `byte`, `i16`, `i32`, `i64`, `enum`
[*floating point*](#floating-point) | `double`, `float`
[*number*](#primitive-types) | *integer*, *floating point*
[*primitive*](#primitive-types) | *number*, `string`, `binary`
[`struct`](#structured-types) | any `struct` type
[`union`](#structured-types) | any `union` type
[`exception`](#structured-types) | any `exception` type
[*structured*](#structured-types) | `struct`, `union`, `exception`
[*container*](#container-types) | `map`, `list`, `set`

## Primitive Types

The basic types supported by Thrift.

Type | Definition
---|---
`bool` | True(1) or False(0)
`byte` | 8-bit signed integer[^1]
`i16` | 16-bit signed integer[^1]
`i32`, `enum` | 32-bit signed integer[^1]
`i64` | 64-bit signed integer[^1]
`float` | 32-bit floating point[^2]
`double` | 64-bit floating point[^2]
`string` | UTF-8 encoded string[^3]
`binary` | Arbitrary byte string

[^1]: [two's complement](https://en.wikipedia.org/wiki/Two%27s_complement)

[^2]: [IEEE-754](https://en.wikipedia.org/wiki/IEEE_754)

[^3]: [UTF-8 encoding](https://en.wikipedia.org/wiki/UTF-8)

Each primitive type defines an intrinsic default:

- numbers → 0
- string, binary → “”

### Enum

Enums are `i32` with named values. Enums *can* represent unnamed values. In ‘open’ enum languages, like C++, this is natively supported. For ‘closed’ enum languages, like Java, special care **must** be taken to allow any value to be read or written to an enum field, even if the value has no name (or the name is not known). Such languages **should** add:

- an explicit `Unrecognized` enum value, which is returned for any unnamed value; and,
- auxiliary methods for get/setting the underlying i32 value.

If no name is declared for the default/0 value, Thrift uses `Unspecified`.

### Floating Point

Thrift considers all `NaN` values to be identical, so custom NaN values **should** be normalized. Additionally, like most programming languages, it is undefined behavior (UB) to have a `NaN` value in a set or map ‘key’ value (even transitively).

### UTF-8 String

While a Thrift `string` **must** always be valid UTF-8, it is treated is identically to `binary`, ignoring all unicode defined semantics, including  which code points are currently defined in the unicode standard. Any well formed unicode encoded code point **may** be present.

## Primitive Operators

### identical

Primitive values are **identical**, if their logical representations are identical. This produces the same results as **equal** in all cases except floating point, which has two exceptions:

    identical(NaN, NaN) == true
    identical(-0.0, 0.0) == false

### equal

Same-type equality follows the semantic rules defined by that type’s specification. This produces the same results as **identical** in all cases except floating point, which has two exceptions:

    equal(NaN, NaN) == false
    equal(-0.0, 0.0) == true

Additionally, values of different numeric types can be tested for equality, in which case they are considered **equal** if they semantically represent equivalent numbers:

    equal(2.0, 2.0f) == equal(2.0f, 2) == true
    equal(-0.0, 0.0f) == equal(-0.0f, 0) == true

**equal** between other primitive types is ill-defined, and not supported.

### empty

A primitive value is considered **empty** if it is **identical** to its intrinsic default.

### hash

A given hash function **must** be able to hash all primitive types directly. All numeric types **must** produce the same hash for **equal** values:

    hash(2.0) == hash(2.0f) == hash(2)
    hash(-0.0) == hash(0.0f) == hash(0)

## Container Types

Type | Definition
---|---
`list<{type}>` | Ordered container of `{type}` values.
`set<{type}>` | Unordered container of unique `{type}` values.
`map<{key}, {value}>` | Unordered container of (`{key}`, `{value}`) pairs, where all `{key}`‘s are unique.

Set and map keys can be any Thrift type; however, storing `NaN` in a key value is not supported and leads to implementation-specific UB (as `NaN` is not **equal** to itself, so it can never be ‘found’).

## Container Operators

All container operators are defined based on values they contain:

- **identical** - Two containers are **identical** if they contain pairwise **identical** elements. For list, the elements must also be in the same order.
- **equal** - Two containers are **equal** if they contain pairwise **equal** elements. For list, the elements must also be in the same order.
- **empty** - A container is **empty** if it contains no elements.
- **hash** - Unordered accumulation, for set and map, and ordered accumulation for list and map key-value pairs, of hashes of all contained elements.

## Structured Types

Type | Definition
---|---
[field](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#Field) | a (id, value) pair.
`struct` | Unordered container of fields. Both field names and ids are unique.
`exception` | Same as `struct`, except can be thrown.
`union` | Similar to `struct`, except at most 1 field can be set.

Structured types have both a 'custom' and 'intrinsic' default value, where the 'intrinsic' default of the structure type ignores all [field](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#Field) custom default values.

[Custom defaults](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#Field.customDefault) are **only** used when constructing a new structured value, accessing the 'default' of a structured type, or parsing a field with [`Fill`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#FieldQualifier.Fill) semantics (the default pre-v1). They are ignored in all other contexts. For example, if a structured value is 'cleared', all of its fields are set to their intrinsic defaults, ignoring all specified custom defaults.

## Structured Operators

Thrift only considers a [field](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#Field)'s `id` and `value` in operators. Similar to `map` operators, structured operators are defined recursively based on the (id, value) pairs of the fields the structured value 'contains':

- **identical** - Two structured values are **identical** if they contain pairwise **identical** field (id, value) pairs.
- **equal** - Two structured values are **equal** if they contain pairwise **equal** field (id, value) pairs.
- **empty** - A structured value is **empty** if all of its fields are **empty**.
- **hash** - Unordered accumulation of, the ordered accumulation of, all non-**empty** field (id, value) pair hashes.

A field is **empty** if:

- it is an [`Optional`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#FieldQualifier.Optional) or `union` field, and the value is 'unset'; or,
- it is a [`Terse`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#FieldQualifier.Terse) field, and the value is **empty**.

When a field is **empty**, no data is written for it; otherwise, the `id` and value are written. When all fields are **empty**, no data is written when serializing the structured value.

**Note** that, a structured type with a [`Fill`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#FieldQualifier.Fill) field (the default before v1), cannot represent an **empty** value, and will **always** output data when serialized.

## Standard Data Types

Types provided by Thrift:

- [type.thrift](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/type.thrift)
- [standard.thrift](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/standard.thrift)
