# Thrift Patch

Thrift Patch implementation **must** contain 2 parts.

1. Functionalities of apply/merge without schema.
2. User-friendly APIs to create/modify/read the patch. Patch created/modified by these APIs must always be valid. User should not interactive with the raw patch struct directly.


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

## Merge

Please refer to the [spec](../spec/definition/patch/#merge-functionality) for the functionality of merge.

Overall, we want to figure out the Patch type first (but it's not strictly necessary),
then we can merge two Patch accordingly.
However, unlike `apply` which we can get patch type from value type,
here we need to deduce Patch type based on PatchOp type. Steps:

1. If `Assign` or `Clear` fields exist in patch2, return patch2.
    1. In this case patch1’s change will be discarded anyway.
2. Otherwise, `Assign` or `Clear` fields don’t exist in patch2. If `Put` field is a bool, we just need to merge two BoolPatch.
    1. We can set `patch1.Put = patch1.Put XOR patch2.Put` and return patch1.
3. Otherwise, if `Add` field is a numeric, we just need to merge two NumericPatch.
    1. We can set `patch1.Add += patch2.Add` and return patch1
4. Otherwise, if `Add` or `Put` field is a string/binary, we just need to merge two StringPatch/BinaryPatch.
    1. We can set `patch1.Add = patch2.Add + patch1.Add` and `patch1.Put += patch2.Put`, then return patch1.
5. Otherwise, if `Add` or `Put` is a list, it can be list patch or set patch.
    1. If `PatchPrior` field exists, it’s a ListPatch. To merge two ListPatch, we need to
        1. Apply `PatchPrior` field in patch2 to `Add` and `Put` field in patch1, since those newly prepend/append elements will be patched immediately in patch2.
        2. Merge `PatchPrior` between patch1 and patch2, recursively.
        3. Similar to string/binary, set `patch1.Add = patch2.Add + patch1.Add` and `patch1.Put += patch2.Put`, then return patch1.
    2. Otherwise, if `Remove` field exists, it’s a set patch. In this case, we can treat `Remove` and `Add` fields as set.
        1. The new `Remove` field should be `patch1.Remove - patch1.Add + patch2.Remove`.
        2. The new `Add` field should be `patch1.Add - patch2.Remove + patch2.Add`.
        3. Here `+` means intersection, `-` means difference in set theory.
    3. Otherwise, set `patch1.Add = patch2.Add + patch1.Add` and `patch1.Put += patch2.Put`, then return patch1. This works for both list and set patch.
6. Otherwise, if `EnsureStruct` field exists, this is a `StructPatch`. In this case
    1. For each field, check whether `patch2.PatchPrior` cleared the field.
        1. If the field is cleared, we can ignore patch1 for this field and copy all PatchOp in patch2 to the final patch (Since whatever changes made by patch1 will be cleared)
    2. Otherwise, check whether this field exists in `patch1.EnsureStruct`.
        1. If so, this field won’t be cleared later, since `patch1.PatchAfter` can’t clear it (see the restriction section), and `patch2.PatchPrior` won’t clear it.
        2. In this case, `patch2.EnsureStruct` will be a no-op for this field, we can merge `patch1.PatchAfter`, `patch2.PatchPrior` and `patch2.PatchAfter` into `PatchAfter` field in merged patch. It’s worth noting `PatchAfter` in merged patch would never clear this field.
    3. Otherwise, `patch1.EnsureStruct` is a no-op for this field, we merge `patch1.PatchPrior`, `patch1.PatchAfter` and `patch2.PatchPrior` into `PatchPrior` field in merged patch.
7. Otherwise, if `EnsureUnion` field exists, this is a `UnionPatch`. In this case
    1. Assume we have exactly one field `X` in `EnsureUnion`, we can add `Clear` field to all fields in `patch2.PatchPrior` if the field is not `X`.
        1. This is because `EnsureUnion` will always clear other fields except `X`.
        2. After this change, the behavior of merged patch would be identical to `StructPatch`, in which case we can just use the same algorithm as `StructPatch`.
8. Otherwise, we can not tell whether the patch is a `StructPatch` or `UnionPatch`. However, we can use the same merge algorithm that we used to merge `StructPatch`, and the result would be the correct regardless whether patch is a `StructPatch` or `UnionPatch`.

If Schema is available, a strongly typed API can be provided to simplify the implementation and detect type mismatch on compile-time.
