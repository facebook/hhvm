# Thrift Patch

## Enable Patch

To enable Patch, `@patch.GeneratePatch` annotation **must** be used recursively on package or struct level.

## Patch Representation

:::info

Patch is an experimental feature. The thrift struct representation is subject to change in the future.

:::

### Patch for Primitive Types

Patch for [primitive types](../../idl/#primitive-types) are defined [here](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/patch.thrift).

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
@thrift.TerseWrite
struct FooFieldPatch {
  1: Type1Patch field1;
  2: Type2Patch field2:
  ...
}

@thrift.TerseWrite
struct FooPatch {
  // Replace Foo with another value
  1: optional Foo assign;

  // Clear all fields in Foo
  2: bool clear;

  // Patch each field in Foo
  3: FooFieldPatch patchPrior;

  // Ensure each field in Foo
  5: FooEnsureStruct ensure;

  // Patch each field in Foo
  6: FooFieldPatch patch;
}
```

Here `FooPatch` is the patch type for `Foo`.

### PatchOp

`PatchOp` is defined [here](https://github.com/facebook/fbthrift/blob/v2023.01.16.00/thrift/lib/thrift/patch.thrift#L262). It represents the meaning of field id in the patch. It’s worth noting that Patch needs to work without schema. When we patch a thrift value, we might not know whether it’s a string patch or a struct patch. With `PatchOp`, we know the meaning of each fields in the patch even without schema.
For the sake of simplifying, in this doc “`Bar` field” means field whoes field id is `PatchOp::Bar`. e.g., `EnsureStruct` field means field whose field id is `PatchOp::EnsureStruct`, which is 5.

### Restriction

A Patch **must** satisfy the following restructions, otherwise it’s considered invalid.

* If `EnsureUnion` field is not empty, it can’t have more than one sub-fields.
* All sub-fields in `PatchAfter` field should not have `clear` PatchOp.

## Apply Functionality

The following functionality should be provided in the target language.

```
void apply(const Object& patch, Value& value);
```

It applies patch to a thrift value and returns the patched value. Note that this API needs to work with dynamic type, thus both input and output are Thrift.Object. To apply the whole Patch, each field in the Patch is applied one by one, ordered by field id ascendingly.

### Complexity

```
O(size of patched fields + size of patch)
```

### Behavior of each `PatchOp` for each value type

|                                    | Assign                | Clear               | PatchPrior                     | EnsureUnion           | EnsureStruct              | PatchAfter         | Remove                              | Add                                           | Put                              |
| ---                                | ---                   | ---                 | ---                            | ---                   | ---                       | ---                | ---                                 | ---                                           | ---                              |
| bool                               | Replace the value[^1] | Clear the value[^2] | N/A                            | N/A                   | N/A                       | N/A                | N/A                                 | N/A                                           | Invert the bool                  |
| byte, i16, i32, i64, float, double |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A                                 | Increase the value by the number in patch     | N/A                              |
| string/binary                      |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A                                 | Prepend the string in the patch to the string | Append the string                |
| list                               |                       |                     | Patch elements in the list     | N/A                   | N/A                       | N/A                | Remove elements that exist in patch | Prepend elements in the patch to the list     | Append elements                  |
| set                                |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | Remove elements that exist in patch | Insert elements in the patch to the set       | Same as Add                      |
| map                                |                       |                     | Patch values in the map[^3]    | N/A                   | Insert key/value pair[^4] | Same as PatchPrior | Remove keys that exist in patch     | N/A                                           | Insert or assign key/value pairs |
| struct/union                       |                       |                     | Patch each field in the struct | Set the active member | Ensure each fields.       | Same as PatchPrior | N/A                                 | N/A                                           | N/A                              |


[^1]: If Assign PatchOp exists, all other PatchOp are ignored.
[^2]: For optional field, clear means reset the field. For elements in container, clear means remove from the container. Otherwise clear means set to intrinstic default.
[^3]: No-op if keys don't exist.
[^4]: No-op if keys exist.

## Merge Functionality

The following functionality should be provided in the target language.

```
Object merge(Object patch1, Object patch2);
```

so that

```
apply(patch1, value)
apply(patch2, value)
```

**must** be equivalent to

```
apply(merge(patch1, patch2), value)
```

Note that this API needs to work with dynamic patch, thus both input and output are Thrift.Object.

### Complexity

```
O(size of patch1 + size of patch2)
```
