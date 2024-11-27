# Thrift Patch

:::note

This is the contributor guide to Thrift Patch, meant for someone implementing Thrift Patch features one of the Thrift Languages. If you're looking for a user guide, see [this page](../fb/features/patch.md).

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

```thrift
struct Foo {
  1: [optional] Type1 field1;
  2: [optional] Type2 field2:
  ...
}
```

Thrift Compiler generates the following structs.

```thrift
// All fields are optional. Original qualifier will be ignored.
struct FooEnsureStruct {
  1: optional Type1 field1;
  2: optional Type2 field2;
}

// All fields are terse. Original qualifier will be ignored.
@thrift.TerseWrite
struct Foo_fbthrift_FieldPatch {
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
  3: Foo_fbthrift_FieldPatch patchPrior;

  // Ensure each field in Foo
  5: FooEnsureStruct ensure;

  // Patch each field in Foo
  6: Foo_fbthrift_FieldPatch patch;
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

```cpp
void apply(const Object& patch, Value& value);
```

It applies patch to a thrift value and returns the patched value. Note that this API needs to work with dynamic type, thus both input and output are Thrift.Object. To apply the whole Patch, each field in the Patch is applied one by one, ordered by field id ascendingly.

### Behavior of each `PatchOp` for each value type

|                                    | Assign                | Clear               | PatchPrior                     | EnsureUnion           | EnsureStruct              | PatchAfter         | Remove          | Add                | Put                              | PatchIfTypeIsPrior (from V2)           | EnsureAny (from V2)           | PatchIfTypeIsAfter (from V2)           |
| ---                                | ---                   | ---                 | ---                            | ---                   | ---                       | ---                | ---             | ---                | ---                              | ---                                    | ---                           | ---                                    |
| bool                               | Replace the value     | Clear the value     | N/A                            | N/A                   | N/A                       | N/A                | N/A             | N/A                | Invert the value                 | N/A                              | N/A                              | N/A                              |
| byte, i16, i32, i64, float, double  |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A             | Increase the value | N/A                              | N/A                              | N/A                              | N/A                              |
| string/binary                      |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A             | Prepend the string | Append the string                | N/A                              | N/A                              | N/A                              |
| list                               |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A             | N/A                | Append elements                  | N/A                              | N/A                              | N/A                              |
| set                                |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | Remove elements  | Insert elements    | N/A                              | N/A                              | N/A                                       | N/A                              |
| map                                |                       |                     | Patch values in the map        | N/A                   | Insert key/value pairs    | Same as PatchPrior | Remove keys      | N/A                | Insert or assign key/value pairs | N/A                              | N/A                                       | N/A                              |
| struct/union                       |                       |                     | Patch fields in the struct      | Set the active member | Ensure fields              | Same as PatchPrior | Remove fields     | N/A                | N/A                              | N/A                              | N/A                                       | N/A                              |
| Thrift Any                         |                       |                     | N/A                            | N/A                   | N/A                       | N/A                | N/A         | N/A                | N/A                              | Patch the value in Thrift Any    | Ensure the type and value in Thrift Any   | Same as PatchIfTypeIsPrior       |


[^1]: If Assign PatchOp exists, all other PatchOp are ignored.
[^2]: For optional field, clear means reset the field. For elements in container, clear means remove from the container. Otherwise clear means set to intrinsic default.
[^3]: No-op if keys don't exist.
[^4]: No-op if keys exist.


### Implementation

1. Use the type of `Value` to figure out the type of `Patch`.
2. Check PatchOps in `Patch` to see whether type matches, throw an exception if type mismatches.
3. Apply each PatchOp one by one, recursively.

The type of each PatchOps in `Patch` is based on Patch type. e.g., for BoolPatch, the `Assign` PatchOp must be `boolean`. Here is the summary of PatchOp's type based on Patch type.

|                                    | Assign        | Clear  | PatchPrior           | EnsureUnion       | EnsureStruct      | PatchAfter           | Remove                            | Add             | Put             | PatchIfTypeIsPrior (from V2)           | EnsureAny (from V2)           | PatchIfTypeIsAfter (from V2)           |
| ---                                | ---           | ---    | ---                  | ---               | ---               | ---                  | ---                               | ---             | ---             | ---                                    | ---                           | ---                                    |
| bool                               | Same of value | `Bool` | N/A                  | N/A               | N/A               | N/A                  | N/A                               | N/A             | `Bool`          | ---                                    | ---                           | ---                                    |
| byte, i16, i32, i64, float, double |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | Same as value   | N/A             | ---                                    | ---                           | ---                                    |
| string/binary                      |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | Same as value   | Same as value   | ---                                    | ---                           | ---                                    |
| list                               |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | N/A             | Same as value   | ---                                    | ---                           | ---                                    |
| set                                |               |        | N/A                  | N/A               | N/A               | N/A                  | `list<Element>` or `set<Element>` | `list<Element>` | `list<Element>` | ---                                    | ---                           | ---                                    |
| map                                |               |        | `map<Key, ValPatch>` | N/A               | Same as value     | `map<Key, ValPatch>` | `list<Key>` or `set<Key>`         | N/A             | Same as value   | ---                                    | ---                           | ---                                    |
| struct/union                       |               |        | `Foo_fbthrift_FieldPatch`      | `FooEnsureStruct` | `FooEnsureStruct` | `Foo_fbthrift_FieldPatch`      | N/A                               | N/A             | N/A             | ---                                    | ---                           | ---                                    |
| Thrift Any                         |               |        | N/A                  | N/A               | N/A               | N/A                  | N/A                               | N/A             | N/A             | `list<TypeToPatchInternalDoNotUse>`    | `any.Any`                     | `list<TypeToPatchInternalDoNotUse>`    |

The corresponding C++ implementation can be found here: [Patch.cpp](https://github.com/facebook/fbthrift/blob/v2023.01.16.00/thrift/lib/cpp2/protocol/Patch.cpp#L126-L164).

If Schema is available, a strongly typed API can be provided to simplify the implementation and detect type mismatch on compile-time.

### Complexity

```
O(size of patched fields + size of patch)
```

## Merge

### Functionality

The following functionality should be provided in the target language.

```cpp
Object merge(Object patch1, Object patch2);
```

so that

```cpp
apply(patch1, value)
apply(patch2, value)
```

**must** be equivalent to

```cpp
apply(merge(patch1, patch2), value)
```

Note that this API needs to work with dynamic patch, thus both input and output are Thrift.Object.

### Complexity

```
O(size of patch1 + size of patch2)
```

## SafePatch

When Thrift Patch is serialized (i.e. sent over the wire or stored), Thrift SafePatch must be used. It provides safe means to transport Thrift Patch over the wire and guarantees that, for any combination of producer and consumer, any serialized instance will either be correctly applied or will fail clearly and deterministically. Thrift SafePatch provides backward compatiblity as well as protection from invalid forward consumption. Thrift SafePatch encodes the minimum Thrift SafePatch version that is required to safely and successfully process the patch. For example, Thrift AnyPatch is available from V2. Even if AnyPatch is available for the binary, if only V1 operations (i.e. `assign`), Thrift SafePatch will encode V1 instead of V2.

## Thrift Patch Changelog
### V2
#### Added
* Support for AnyPatch
