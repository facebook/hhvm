---
state: draft
---

# Values

How Thrift values are declared, evaluated, validated, and manipulated.

## Consts and Defaults

How const and default values function in Thrift.

## Properties

The properties that can be evaluated for all Thrift values.

### Equal

If two values should be considered equivalent. For example `-0.0` is equal to, but not identical to `0.0`.

### Identical

If two values have identical representations. For example `NaN` is identical to, but not equal to itself.

### Empty

If a value contains no information (e.g. it is equal to `[]`/`{}`/`0`/`""`, etc), and would be omitted in a 'terse' context.

## Operators

The operators that can be applied to all Thrift values.

### Clear

Sets a value to the types intrinsic default, ignoring any 'custom' defaults.

### Hash

Computes a deterministic digest of a value.
