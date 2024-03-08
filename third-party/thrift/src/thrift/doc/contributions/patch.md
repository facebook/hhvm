# Thrift Patch

:::note

This is the contributor guide to Thrift Patch, meant for someone implementing Thrift Patch features one of the Thrift Languages. If you're looking for a user guide, see [this page](../features/patch/patch.md).

:::

Thrift Patch implementation **must** contain 2 parts.

1. Functionalities of apply/merge without schema.
2. User-friendly APIs to create/modify/read the patch. Patch created/modified by these APIs must always be valid. User should not interactive with the raw patch struct directly.

## The High-level Implementation Of Thrift Patch

Thrift Patch describes how Thrift value is mutated. Logically, it's a list of operations where each operation describes a single change to the Thrift value.

Thrift Patch is implemented in an unconventional way — it’s a fixed-length list of operations and the type of operations are pre-arranged in a way that we can merge two patches into a single patch.

For example, if `SetPatch` contains two types of operations — adding an element or removing an element, it can be implemented this way in C++.

```cpp
template<class E>
class SetPatch {
 private:
  enum class Op { Add, Remove };
  std::vector<std::pair<Op, E>> operations_;

 public:
  void add(E e) { operations_.emplace_back(Op::Add, e); }
  void remove(E e) { operations_.emplace_back(Op::Remove, e); }
  void apply(std::set<E>& v) {
    for (auto [op, e]: operations_) {
      op == Op::Add ? v.insert(e) : v.erase(e);
    }
  }
}
```

So that we can loop over all operations and apply them to an actual set. However, the Thrift Patch implementation is similar to this:

```cpp
template<class E>
class SetPatch {
 private:
  std::set<E> add_, remove_;

 public:
  void add(E e) { remove_.erase(e), add_.insert(e); }
  void remove(E e) { add_.erase(e), remove_.insert(e); }
  void apply(std::set<E>& v) {
    for (auto i: add_) { v.insert(i); }
    for (auto i: remove_) { v.erase(i); }
  }
}
```

:::note

This is a high-level illustration. It is not the actual implementation.

:::

It only has two operations — inserting elements, and then removing elements. It doesn't use a list, but it has the same behavior as previous implementation. This is how Patch should be implemented in all languages.

* **Pros**: In general, this provides better performance. In fact, for Numeric Types (integers and float pointers), the patch is guaranteed to have `O(1)` time/space when applying it, regardless how many operations we have.
* **Cons**: We cannot create an arbitrary new operation. Any new operation we have added must be compatible with all existing ones. Basically, when a user adds an operation to the Patch, we need to be able swap them to the desired order without relying on the patched data (when creating the patch, we don’t know the patched data yet).

For example, in the case above, the desired order is `add` elements first, then `remove` elements.

If a user wants `remove(foo)` first, then `add(bar)`, we can reorder to `add(bar)`, then `remove(foo - bar)`, which is the desired order without changing the behvaior.

Now consider if we want to create a new operation -- `remove_smallest()` -- that removes the smallest element in the set. We cannot add such operation without changing existing ones, since this operation is not compatible with `add`. We cannot swap `add(foo)` and `remove_smallest()` (as we do not know whether the smallest element is in `foo`, or the original list), vice versa. If we used the alternative approach to implement Patch as a list of operations, we wouldn't have the problem to create such new operation.

On the other hand, if we want to create a new operation -- `clear()` -- that removes all elements, both `add(foo), clear()` and `remove(foo), clear()` can be replaced by just `clear()`, since once we cleared everything, it doesn't matter what we added/removed. We can change the implementation to

```cpp
template<class E>
class SetPatch {
 private:
  bool clear_;
  std::set<E> add_, remove_;

 public:
  void add(E e) { remove_.erase(e), add_.insert(e); }
  void remove(E e) { add_.erase(e), remove_.insert(e); }
  void clear() { add_.clear(), remove_.clear(), clear_ = true; }
  void apply(std::set<E>& v) {
    if (clear_) { v.clear(); }
    for (auto i: add_) { v.insert(i); }
    for (auto i: remove_) { v.erase(i); }
  }
}
```

The `clear` operation must be applied first, otherwise it won't work (Why? `add(foo), clear()` can be replaced with `clear(), add({})`, but `clear(), add(foo)` cannot be replaced with `add(something), clear()`).

## Patch Representation

:::info

Patch is an experimental feature. The thrift struct representation is subject to change in the future.

:::

### Patch for Primitive Types

Patch for [primitive types](../idl/#primitive-types) are defined [here](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/patch.thrift).

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

## Apply

### Functionality

The following functionality should be provided in the target language.

```
void apply(const Object& patch, Value& value);
```

It applies patch to a thrift value and returns the patched value. Note that this API needs to work with dynamic type, thus both input and output are Thrift.Object. To apply the whole Patch, each field in the Patch is applied one by one, ordered by field id ascendingly.

### Behavior of each `PatchOp` for each value type

|                                    | Assign                | Clear               | PatchPrior                     | EnsureUnion           | EnsureStruct              | PatchAfter         | Remove          | Add                | Put                              |
| ---                                | ---                   | ---                 | ---                            | ---                   | ---                       | ---                | ---             | ---                | ---                              |
| bool                               | Replace the value     | Clear the value     | N/A                            | N/A                   | N/A                       | N/A                | N/A             | N/A                | Invert the value                 |
| byte, i16, i32, i64, float, double  |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A             | Increase the value | N/A                              |
| string/binary                      |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A             | Prepend the string | Append the string                |
| list                               |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A             | N/A                | Append elements                  |
| set                                |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | Remove elements | Insert elements    | N/A                              |
| map                                |                       |                     | Patch values in the map        | N/A                   | Insert key/value pairs    | Same as PatchPrior | Remove keys     | N/A                | Insert or assign key/value pairs |
| struct/union                       |                       |                     | Patch fields in the struct      | Set the active member | Ensure fields              | Same as PatchPrior | Remove fields    | N/A                | N/A                              |


[^1]: If Assign PatchOp exists, all other PatchOp are ignored.
[^2]: For optional field, clear means reset the field. For elements in container, clear means remove from the container. Otherwise clear means set to intrinsic default.
[^3]: No-op if keys don't exist.
[^4]: No-op if keys exist.


### Implementation

1. Use the type of `Value` to figure out the type of `Patch`.
2. Check PatchOps in `Patch` to see whether type matches, throw an exception if type mismatches.
3. Apply each PatchOp one by one, recursively.

The type of each PatchOps in `Patch` is based on Patch type. e.g., for BoolPatch, the `Assign` PatchOp must be `boolean`. Here is the summary of PatchOp's type based on Patch type.

|                                    | Assign        | Clear  | PatchPrior           | EnsureUnion       | EnsureStruct      | PatchAfter           | Remove                            | Add             | Put             |
| ---                                | ---           | ---    | ---                  | ---               | ---               | ---                  | ---                               | ---             | ---             |
| bool                               | Same of value | `Bool` | N/A                  | N/A               | N/A               | N/A                  | N/A                               | N/A             | `Bool`          |
| byte, i16, i32, i64, float, double |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | Same as value   | N/A             |
| string/binary                      |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | Same as value   | Same as value   |
| list                               |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | N/A             | Same as value   |
| set                                |               |        | N/A                  | N/A               | N/A               | N/A                  | `list<Element>` or `set<Element>` | `list<Element>` | `list<Element>` |
| map                                |               |        | `map<Key, ValPatch>` | N/A               | Same as value     | `map<Key, ValPatch>` | `list<Key>` or `set<Key>`         | N/A             | Same as value   |
| struct/union                       |               |        | `FooFieldPatch`      | `FooEnsureStruct` | `FooEnsureStruct` | `FooFieldPatch`      | N/A                               | N/A             | N/A             |

The corresponding C++ implementation can be found here: [Patch.cpp](https://github.com/facebook/fbthrift/blob/v2023.01.16.00/thrift/lib/cpp2/protocol/Patch.cpp#L126-L164).

If Schema is available, a strongly typed API can be provided to simplify the implementation and detect type mismatch on compile-time.

### Complexity

```
O(size of patched fields + size of patch)
```

## Merge

### Functionality

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

### Implementation

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

### Complexity

```
O(size of patch1 + size of patch2)
```
