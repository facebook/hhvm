# Thrift Patch

Thrift Patch implementation **should** contain 2 parts.

1. Functionalities of apply/merge without schema as minimal support.
2. More user-friendly APIs to create/modify the patch. Patch created/modified by these APIs must always be valid.

A conforming implementation **must** at least implement apply/merge without schema.

## Apply

Please refer to the [spec](../spec/definition/patch/#apply-functionality) for the functionality of apply. Steps:

1. Use the type of `Value` to figure out the type of `Patch`.
2. Check PatchOps in `Patch` to see whether type matches, throw an exception if type mismatches.
3. Apply each PatchOp one by one, recursively.

The type of each PatchOps in `Patch` is based on Patch type. e.g., for BoolPatch, the `Assign` PatchOp must be `boolean`. Here is the summary of PatchOp's type based on Patch type.

|                                    | Assign        | Clear  | PatchPrior           | EnsureUnion       | EnsureStruct      | PatchAfter           | Remove                            | Add             | Put             |
| ---                                | ---           | ---    | ---                  | ---               | ---               | ---                  | ---                               | ---             | ---             |
| bool                               | Same of value | `Bool` | N/A                  | N/A               | N/A               | N/A                  | N/A                               | N/A             | `Bool`          |
| byte, i16, i32, i64, float, double |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | Same as value   | N/A             |
| string/binary                      |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | Same as value   | Same as value   |
| list                               |               |        | `list<ElementPatch>` | N/A               | N/A               | N/A                  | `list<Element>` or `set<Element>` | `list<Element>` | `list<Element>` |
| set                                |               |        | N/A                  | N/A               | N/A               | N/A                  | `list<Element>` or `set<Element>` | `list<Element>` | `list<Element>` |
| map                                |               |        | `map<Key, ValPatch>` | N/A               | Same as value     | `map<Key, ValPatch>` | `list<Key>` or `set<Key>`         | N/A             | Same as value   |
| struct/union                       |               |        | `FooFieldPatch`      | `FooEnsureStruct` | `FooEnsureStruct` | `FooFieldPatch`      | N/A                               | N/A             | N/A             |

The corresponding C++ implementation can be found here: [Patch.cpp](https://github.com/facebook/fbthrift/blob/v2023.01.16.00/thrift/lib/cpp2/protocol/Patch.cpp#L126-L164).

If Schema is available, a strongly typed API can be provided to simplify the implementation and detect type mismatch on compile-time.
