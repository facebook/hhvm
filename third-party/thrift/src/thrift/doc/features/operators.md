# Operators

Operators allow evaluating, validating, or manipulating thrift values.

## Primitive Operators

Defines operators which can be applied to [primitive types](../idl/#primitive-types) and [enums](../idl/#enumeration-types).

### identical

Primitive values are **identical**, if their logical representations are identical. This produces the same results as **equal** in all cases except floating point, which has two exceptions:

    identical(NaN, NaN) == true
    identical(-0.0, 0.0) == false

Thrift considers all `NaN` values to be identical, so custom NaN values **should** be normalized. While a Thrift `string` **must** always be valid UTF-8, it is treated as identically to `binary`, ignoring all unicode defined semantics, including  which code points are currently defined in the unicode standard. Any well formed unicode encoded code point **may** be present.

### equal

Same-type equality follows the semantic rules defined by that type’s specification. This produces the same results as **identical** in all cases except floating point, which has two exceptions:

    equal(NaN, NaN) == false
    equal(-0.0, 0.0) == true

Additionally, values of different numeric types can be tested for equality, in which case they are considered **equal** if they semantically represent equivalent numbers:

    equal(2.0, 2.0f) == equal(2.0f, 2) == true
    equal(-0.0, 0.0f) == equal(-0.0f, 0) == true

**equal** between other primitive types is ill-defined, and not supported.

### empty

A primitive value is considered **empty** if it is **identical** to its intrinsic default and would be omitted in a 'terse' context.

### clear

Sets a value to the types intrinsic default, ignoring any 'custom' defaults.

### hash

Computes a deterministic digest of a value. A given hash function **must** be able to hash all primitive types directly. All numeric types **must** produce the same hash for **equal** values:

    hash(2.0) == hash(2.0f) == hash(2)
    hash(-0.0) == hash(0.0f) == hash(0)

## Container Operators

All [container](../idl/#container-types) operators are defined based on values they contain:

- **identical** - Two containers are **identical** if they contain pairwise **identical** elements. For list, the elements must also be in the same order.
- **equal** - Two containers are **equal** if they contain pairwise **equal** elements. For list, the elements must also be in the same order.
- **empty** - A container is **empty** if it contains no elements.
- **clear** - Removes all of the elements from the container.
- **hash** - Unordered accumulation, for set and map, and ordered accumulation for list and map key-value pairs, of hashes of all contained elements.

Set and map keys can be any Thrift type; however, storing `NaN` in a key value is not supported and leads to implementation-specific undefined behavior (as `NaN` is not **equal** to itself, so it can never be ‘found’).

## Structured Operators

Structured operators can be applied to [struct](../idl/#struct-types), [union](../idl/#union-types) and [exception](../idl/#exception-types). Thrift only considers a [field](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#Field)'s `id` and `value` in operators. Similar to `map` operators, structured operators are defined recursively based on the (id, value) pairs of the fields the structured value 'contains':

- **identical** - Two structured values are **identical** if they contain pairwise **identical** field (id, value) pairs.
- **equal** - Two structured values are **equal** if they contain pairwise **equal** field (id, value) pairs.
- **empty** - A structured value is **empty** if all of its fields are **empty**.
- **clear** - Sets all values to the types intrinsic default, ignoring any 'custom' defaults.
- **hash** - Unordered accumulation of, the ordered accumulation of, all non-**empty** field (id, value) pair hashes.

A field is **empty** if:

- it is an [`Optional`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#FieldQualifier.Optional) or `union` field, and the value is 'unset'; or,
- it is a [`Terse`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#FieldQualifier.Terse) field, and the value is **empty**.

When a field is **empty**, no data is written for it; otherwise, the `id` and value are written. When all fields are **empty**, no data is written when serializing the structured value.

**Note** that, a structured type with a [`Fill`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/schema.thrift#FieldQualifier.Fill) field, cannot represent an **empty** value, and will **always** output data when serialized.
