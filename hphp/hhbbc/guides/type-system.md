# HHBBC Type System

> Sections marked with 📋 list specific names that may drift as code evolves. If a name isn't found, search for the pattern nearby.

## Architecture

A Type is `trep` (bitset) + optional `DataTag` specialization + `LegacyMark`.

**`trep`** (`type-system-bits.h`): Bitset of possible runtime types. Examples:
`BInt`, `BStr`, `BObj`, `BVec`, `BDict`, `BNull`, `BBottom`, `BCell` (= all).

**Specialization**: Only ONE DataTag at a time. The specialization must be
"supported" by exactly one supportful trep bit being present — you can't have
both `BStr` and `BInt` set with a specialization.

### 📋 DataTag variants

| Tag | Applies to | What it carries |
|-----|-----------|-----------------|
| `DCls` | `BObj` | Exact/sub/isect class, ctx tag, nonRegular flag |
| `DWaitHandle` | `BObj` (WaitHandle subclass) | Inner awaited type |
| `DArrLikePacked` | `BVecN`/`BDictN`/`BKeysetN` | Per-element types |
| `DArrLikePackedN` | same | Single element type (all same) |
| `DArrLikeMap` | same | Known key→type pairs + optional rest k/v |
| `DArrLikeMapN` | same | Key type + value type (all unknown) |
| `DStr` | `BSStr` | Exact static string value |
| `DInt` | `BInt` | Exact integer value |
| `DDbl` | `BDbl` | Exact double value |
| `DLazyCls` | `BLazyCls` | Exact lazy class value |
| `DEnumClassLabel` | `BEnumClassLabel` | Exact enum class label string |

**Array specializations only apply to non-empty arrays** (`BArrLikeN`), not
`BArrLike` (which includes empty). The specialization describes only the
non-empty portion.

## Key Operations

```cpp
// Extract constant value (returns nullopt for counted types)
auto const val = tv(type);        // std::optional<TypedValue>
auto const val = tvCounted(type); // ignores countedness

// Create type from constant value (pre: non-refcounted cell)
auto const t = from_cell(tv);

// Subtype check
if (a.subtypeOf(b)) { ... }   // a ⊆ b
if (a.couldBe(b)) { ... }     // a ∩ b ≠ ∅

// Set operations
auto const u = union_of(a, b);        // a ∪ b
auto const i = intersection_of(a, b); // a ∩ b

// CRITICAL: use widening_union in dataflow fixed points
auto const w = widening_union(a, b);  // guarantees termination
```

**`operator==` is deliberately NOT implemented on Type.** Use `equal()` or
`equalNoContext()` explicitly.

## Widening and Loosening

`union_of` on array types may NOT reach a fixed point in finite steps — you
**must** use `widening_union` in dataflow analysis to guarantee termination.

The `loosen_*` family models COW, iterator effects, and imprecision:
- `loosen_staticness` — static → maybe-counted
- `loosen_values` — drops constant values, keeps structure
- `loosen_emptiness` — non-empty → maybe-empty
- `loosen_array_staticness` — loosen staticness within array elements

## DCls (Class Specialization)

`DCls` packs exact/sub/isect class info, ctx tag, and nonRegular flag into a
`CompactTaggedPtr`. Intersection classes must have 2+ elements, be in canonical
order, all `couldBe` each other, and contain no redundant supertypes.

`Class::combine()` and `Class::intersect()` implement set-theoretic operations
for `union_of`/`intersection_of` on object types.

## Pitfalls

- **Check `type-system.h` for the ASCII lattice diagram** before working with the type system
- **Only one specialization at a time** — can't have both array and class specialization
- **Array specialization requires exactly one supportful trep** — can't specialize a union like `BVecN|BDictN`
- **Use `widening_union` not `union_of`** in fixed-point loops
- **No `operator==`** — this is deliberate, use `equal()` / `equalNoContext()`

Key files: `type-system.h`, `type-system-bits.h`, `type-system.cpp`.
