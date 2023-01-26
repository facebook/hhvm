# Thrift Patch

## Enable Patch

To enable Patch, `@patch.GeneratePatch` annotation **must** be used recursively on package or struct level.

## Patch Representation

:::info

Patch is an experimental feature. The thrift struct representation is subject to change in the future.

:::

### Patch for Primitive Types

Patch for [Primitive types](data.md#primitive-types) are defined [here](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/patch.thrift).

### Patch for Structs

Thrift Compiler generates multiple patch structures for a given thrift struct. Considering the following thrift struct

```
struct Foo {
  1: [optional] Type1 field1;
  2: [optional] Type2 field2:
  ...
}
```

Thrift Compiler generates the following structs.

```
// All fields are optional. Original qualifier will be ignored.
struct FooEnsureStruct {
  1: optional Type1 field1;
  2: optional Type2 field2;
}

// All fields are terse. Original qualifier will be ignored.
struct FooFieldPatch {
  @thrift.TerseWrite
  1: Type1Patch field1;
  @thrift.TerseWrite
  2: Type2Patch field2:
  ...
}

struct FooPatch {
  // Replace Foo with another value
  1: optional MyStruct assign;

  // Clear all fields in Foo
  @thrift.TerseWrite
  2: bool clear;

  // Patch each field in Foo
  @thrift.TerseWrite
  3: FooFieldPatch patchPrior;

  // Ensure each field in Foo
  @thrift.TerseWrite
  5: FooEnsureStruct ensure;

  // Patch each field in Foo
  @thrift.TerseWrite
  6: FooFieldPatch patch;
}
```

Here `FooPatch` is the Patch for `Foo`.

### PatchOp

PatchOp is defined [here](https://github.com/facebook/fbthrift/blob/v2023.01.16.00/thrift/lib/thrift/patch.thrift#L262). It represents the meaning of field id in the patch. It’s worth noting that Patch needs to work without schema. When we patch a thrift value, we might not know whether it’s a string patch or a struct patch. With `PatchOp`, we know the meaning of each fields in the patch even without schema.
For the sake of simplifying, in this doc “`Bar` field” means field whoes field id is `PatchOp::Bar`. e.g., `EnsureStruct` field means field whose field id is `PatchOp::EnsureStruct`, which is 5.

### Restriction

A Patch **must** satisfy the following restructions, otherwise it’s considered invalid.

* If `EnsureUnion` field is not empty, it can’t have more than one sub-fields.
* All sub-fields in `PatchAfter` field should not have `clear` PatchOp.
