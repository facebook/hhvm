## Shape Splat \+ Shape Polymorphism

## Summary

**TL;DR**: Hack has no mechanism to compose and decompose shape types or a generic way to operate on parts of a composed shape type. With the features proposed in this HIP, we enable composition, accurate typing of expression level ‘merge’ operations and support for general shape polymorphic functions:

```c#
// Composition (§1.1): type aliases that build on each other, rightmost-wins semantics
type User = shape('id' => arraykey, 'name' => string, 'age' => int);
type TimestampedUser = shape(...User, 'created_at' => int);

// Precise merge (§1.2): rightmost-wins semantics, mirroring type level composition
$config = shape(...$defaults, ...$overrides);

// Polymorphism (§1.3): modify known fields, preserve the rest
function update_name<T as shape(mixed...)>(
  shape(...T, 'name' => string) $s,
): shape(...T, 'name' => string) {
  $s['name'] = $s['name'].'!';
  return $s;
}
```

## 1\. Motivation

Hack's shape types provide structural typing for record-like values:

```c#
function get_name(shape('name' => string, 'age' => int) $user): string {
  return $user['name'];
}
```

You can name shapes with type aliases to avoid repetition:

```c#
type User = shape('id' => arraykey, 'name' => string, 'age' => int);
```

And open shapes let you write functions that accept shapes with extra, unknown fields beyond those listed:

```c#
function get_name(shape('name' => string, ...) $s): string {
  return $s['name'];
}
```

Despite their flexibility, shapes have three significant typing limitations.

### 1.1 Shape type aliases cannot be composed

Suppose you want a `TimestampedUser` that has all the fields of `User` plus a `'created_at'` field. You cannot write this:

```c#
// NOT valid Hack today.
type TimestampedUser = shape(...User, 'created_at' => int);
```

Instead, you must copy every field from `User` into the new alias:

```c#
type User = shape('id' => arraykey, 'name' => string, 'age' => int);

// Must duplicate all of User's fields:
type TimestampedUser =
  shape('id' => arraykey, 'name' => string, 'age' => int, 'created_at' => int);
```

This is brittle. If someone adds an `'email'` field to `User`, they must remember to update `TimestampedUser`, and every other alias that was copy-pasted from it.

### 1.2 Shape merge cannot be accurately typed

At the value level, merging two shapes is a common operation, but the available options lose type information.

`Shape\fb\merge` merges a list of open shapes from left-to-right, overwriting fields from earlier shapes. A slightly simplified version which takes two parameters is shown below:

```c#
function merge2<T as shape(...)>(T $first, T $second)[]: T {
  $as_array = Shapes::toArray($first);
  foreach (Shapes::toArray($second) as $k => $v) {
    $as_array[$k] = $v;
  }
  return HH\FIXME\UNSAFE_CAST<dict<arraykey, mixed>, T>($as_array);
}
```

This function has a single type parameter `T` with upper bound `shape(...)`, the top shape type. This means we can pass arguments with any shape type and `T` will be instantiated with the least upper bound of the types. In practice, this means all fields which are not in the intersection of the two shapes will be optional:

```c#
function merge_closed(
  shape('x' => int, 'y' => bool) $has_x_y,
  shape('y' => int, 'z' => bool) $has_y_z,
): mixed {
  // shape(?'x' => int, 'y' => (int | bool), ?'z' => bool)
  $out = merge2($has_x_y, $has_y_z);
  return $out;
}
```

We get this type because absent fields can be viewed as (or even encoded as) optional fields with type `nothing`. Applying union structurally:

```
x: (Req, int)     ∨ (Opt, nothing) = (Opt, int)
y: (Req, bool)    ∨ (Req, int)     = (Req, bool | int)
z: (Opt, nothing) ∨ (Req, bool)    = (Opt, bool)
```

Given that the function overwrites fields from the first shape with the corresponding field from the second, the accurate return type is `shape('x' => int, 'y' => int, 'z' => bool)`, but there is no way to express this polymorphically in Hack.

The situation is worse when the shapes are open:

```c#
function merge_open(
  shape('x' => int, 'y' => bool, ...) $has_x_y,
  shape('y' => int, 'z' => bool, ...) $has_y_z,
): mixed {
  // shape(?'x' => mixed, 'y' => (int | bool), ?'z' => mixed, ...)
  $out = merge2($has_x_y, $has_y_z);
  return $out;
}
```

The accurate type is `shape('x' => int, 'y' => int, 'z' => bool, ...)` but now we end up with an even larger supertype where non-common fields become `?'x' => mixed`.

The key takeaway: rightmost-wins shape merging is not described by a union operation on shape types or any other existing part of Hack's type system.

### 1.3 Shape polymorphism is limited

Imagine that Hack already supported shape merging and that we could compose type aliases:

```c#
type User = shape('id' => arraykey, 'name' => string, 'age' => int);
type TimestampedUser = shape(...User, 'created_at' => int);
```

It seems reasonable to expect that we could write functions which work for both `User` and `TimestampedUser` rather than copy-pasting a version for each type.

When we try to write such a function, we run into problems:

```c#
function update_user<
  T as shape('id' => arraykey, 'name' => string, 'age' => int, ...),
>(T $input): T {
  $input['id'] = $input['id'].'!';
  $input['name'] = $input['name'].'!';
  $input['age'] = $input['age'] + 1;
  // Error: shape('id' => string, 'age' => int, 'name' => string, ...) </: T
  return $input;
}
```

The type parameter `T` is fixed but known only up to its bounds in the body of the function. We know `T <: shape('name' => string, 'age' => int, ...)`, that is, `T` is some specific shape with at least those fields, but it could have more (and could be closed). For example, `T` might be `shape('name' => string, 'age' => int, 'email' => string)`.

During the field assignments, Hack re-types `$input` as the upper bound: `shape('name' => string, 'age' => int, ...)`. It forgets the value was originally a `T`. This is the only thing it can soundly do in the general case (see §4.3.2 for the detailed typing analysis).

Now, the return type needs `shape('name' => string, 'age' => int, ...) <: T`, but we only know the reverse: `T <: shape('name' => string, 'age' => int, ...)`.

### 1.4 What we want to write

The three limitations above share a common theme: Hack lacks a way to compose and decompose shape types. With the features proposed in §2, all three become expressible:

```c#
// Composition (§1.1): type aliases that build on each other
type User = shape('id' => arraykey, 'name' => string, 'age' => int);
type TimestampedUser = shape(...User, 'created_at' => int);

// Precise merge (§1.2): each operand keeps its own type
$config = shape(...$defaults, ...$overrides);

// Polymorphism (§1.3): modify known fields, preserve the rest
function update_name<T as shape(mixed...)>(
  shape(...T, 'name' => string) $s,
): shape(...T, 'name' => string) {
  $s['name'] = $s['name'].'!';
  return $s;
}
```

## 2\. Proposal

We propose extending Hack's shape types with three features that address the limitations identified in §1:

- **Shape splats** (§2.1) address type alias composition (§1.1) and shape merge typing (§1.2).
- **Shape polymorphism** (§2.2) addresses limited shape polymorphism (§1.3).
- **`absent` field declarations** (§2.3) are a supporting mechanism that make splats and polymorphism safe and expressive.

### 2.1 Shape Splats

The **shape splat** operator `...` composes shapes at both the type and value level. At the type level, it enables type alias composition (§1.1):

```c#
type User = shape('id' => arraykey, 'name' => string, 'age' => int);
type TimestampedUser = shape(...User, 'created_at' => int);
// Equivalent to shape('id' => arraykey, 'name' => string, 'age' => int, 'created_at' => int)
```

At the value level, it enables type-safe shape merging (§1.2):

```c#
$result = shape(...$defaults, ...$overrides);
```

When two splat elements contribute the same field, the rightmost one wins, matching the runtime semantics of `Shapes::merge`. Unlike `Shape\fb\merge`, each operand keeps its own type and the result type is computed precisely. See §3 for the full syntax and §4.2 for the typing rules.

### 2.2 Shape Polymorphism

Shape splats combine with generic type parameters to enable **polymorphic shape functions** that preserve the caller's fields through operations on known fields (§1.3):

```c#
function update_user<T as shape(mixed...)>(
  shape(...T, 'name' => string, 'age' => int) $input,
): shape(...T, 'name' => string, 'age' => int) {
  $input['name'] = $input['name'].'!';
  $input['age'] = $input['age'] + 1;
  return $input;
}
```

The type parameter `T` captures "the rest of the fields", the ones we are not modifying. The concrete fields `'name'` and `'age'` are separated out so they can be modified without losing `T`'s identity. See §4.4.3 for the subtyping rules and §4.5 for inference.

### 2.3 The Absent Field

The **absent field,** `absent 'x'`, explicitly declares that a field is not present (desugars to `?'x' => nothing`). This is primarily useful on type parameter constraints, to exclude specific fields from the unknown part of a shape:

```c#
function add_name<T as shape(absent 'name', mixed...)>(
  shape(...T) $s, string $name,
): shape(...T, 'name' => string) {
  return shape(...$s, 'name' => $name);
}
```

The constraint `absent 'name'` ensures `T` does not already have a `'name'` field, so the concrete `'name' => string` in the return type is unambiguously the one we added.

**Note:** `absent` does not *remove* fields from concrete shapes. Under rightmost-wins merge, `shape(...User, absent 'age')` leaves `User`'s required `'age'` field intact (see §4.1.4). A dedicated `remove` keyword for field deletion is future work (see §4.1.4).

### 2.5 Key Design Decisions

| Decision | Our approach | Alternative considered | Rationale |
| :---- | :---- | :---- | :---- |
| Overlap semantics | Rightmost-wins | Reject overlapping fields | Matches Hack's value-level `Shapes::merge`; enables defaults pattern |
| Disjointness | `absent` fields;  opt-in constraint as future work | Always require disjoint fields | Rightmost-wins is useful; `absent` available when safety is needed Full `disjoint_field` constraints would give us full static guarantees as an opt-in |
| Extension points | Multiple splats | Single extension variable per shape | More expressive: `shape(...T, 'x' => int, ...U)` |
| Field absence | `absent 'x'` \= `?'x' => nothing` | Separate presence/absence flag per field | Reuses existing Opt/nothing lattice |
| Open shapes | `unknown` type | Extension variable for unknown fields | Generalises open/closed uniformly |
| Syntax | Mirror tuple splat syntax | N/A | The features are closely related  |

These alternatives correspond to row-type systems (Wand, Rémy); see §7 for a detailed comparison.

### 2.6 Delivery

These features will be introduced in a phased manner under the experimental feature flag `shape_splat`:

1. **Phase 1: Type-level splats.** Shape splat syntax in type aliases and function signatures. Type alias composition, normalization, subtyping for concrete splats.
2. **Phase 2: Polymorphic shape functions.** Inference for flex variables in shape splats. Well-formedness checking (sole-splat condition).
3. **Phase 3: Expression-level splats.** Value-level `shape(...$a, ...$b)` syntax with runtime support.

### 2.7 Non-Goals

The following are explicitly out of scope for this proposal:

- **Shape destructuring.** Destructuring shape values into local bindings (e.g., `shape('x' => $x, ...$rest) = $s`) is the subject of a separate proposal.
- **Field removal from concrete compositions.** The `absent` keyword does not remove fields from concrete shapes (§4.1.4). A dedicated `remove` keyword is future work.
- **Static, polymorphic disjointness.** A `disjoint_fields` constraint would give us an opt-in way of recovering the behavior of ‘simple rows’ which may be desirable for some use cases where users want to statically ensure merge doesn’t overwrite existing fields.
- **Label polymorphism.** Some patterns of use of shapes are not expressible statically under the features added by this proposal, for example:
  - Given any shape type, return the shape but with all fields with requiredness `Req` now `Opt`
  - Given any shape type, return the shape but will all types made nullable
  - Given any closed shape type, return the corresponding open shape type

  In each case, you can write a non-polymorphic function which does this trivially (since the before and after types are subtypes) but they can’t be expressed polymorphically without operating on labels.

### 2.8 Document Overview

The remainder of this document specifies the proposal in detail:

- **§3 Syntax**: the surface syntax changes in EBNF.
- **§4 Typing**: type representation, expression typing, subtyping rules, inference and well-formedness conditions. Includes traced examples from the mousetrap prototype.
- **§5 Runtime**: HHVM runtime and emitter changes (preliminary positions).
- **§6 Other considerations**: type constants, HackAST integration, error messages, IDE support, migration.
- **§7 Prior art**: comparison with row types, record concatenation, and related type systems.

## 3\. Syntax

Hack already has shape types, open shapes, and optional fields. This proposal adds new syntactic forms, shown below alongside the existing grammar. The syntax is chosen to be consistent with the existing tuple splats feature.

**Note**: today's open shape syntax `shape('x' => int, ...)` uses a standalone trailing `...`. In the proposed grammar, `...` retains this meaning (equivalent to `mixed...`). When `...` is followed by a type, it is a splat. Throughout this document, we use the explicit `shape(mixed...)` form for open shapes to avoid ambiguity with splat syntax.

### 3.1 Existing Hack Shape Syntax

```
shape_type  ::= 'shape' '(' field_list? '...'? ')'
field_list  ::= field (',' field)* ','?
field       ::= '?'? string_literal '=>' type
```

Examples:

```
shape('name' => string, 'age' => int)        // closed shape
shape('name' => string, ...)                  // open shape (unknown fields allowed)
shape(?'name' => string)                      // optional field
```

### 3.2 Proposed Additions

New productions are marked with ★. Existing productions are included for context.

```
shape_type    ::= 'shape' '(' element_list? ')'                        -- ★ extended

element_list  ::= element (',' element)* ','?

element       ::= field                                                -- existing
                | '...' type                                           -- ★ type-level splat
                | '...'                                                -- existing (open shape)

field         ::= '?'? field_name '=>' type                            -- existing
                | 'absent' field_name                                  -- ★ absent field

field_name    ::= string_literal                                       -- 'x', 'name', ...
                | 'nameof' class_name                                  -- nameof Foo
                | class_name '::' const_name                           -- Foo::BAR

shape_expr    ::= 'shape' '(' expr_element_list? ')'                   -- ★ extended

expr_element  ::= field_name '=>' expr                                 -- existing
                | variable                                             -- existing (punning)
                | '...' expr                                           -- ★ value-level splat
```

**Parser disambiguation.** The `...` token is ambiguous between "open shape" (existing) and "splat" (new). The parser resolves this by context:

- In a **type position** (type alias body, function signature), `...` followed by a type name is a type-level splat; `...` at the end of the element list with no following type is an open shape.
- In an **expression position** (shape literal), `...` followed by an expression is a value-level splat. A standalone trailing `...` is not valid in expression position (shape literals are always closed).

This is unambiguous because type names and expressions occupy distinct syntactic categories, and the trailing `...` (open shape) can only appear as the last element.

### 3.3 Description of syntax additions

**Type-level splat** (`...T`): splat a type parameter or type alias into a shape type. Fields and splats can be freely interleaved in surface syntax:

```c#
type TimestampedUser = shape(...User, 'created_at' => int);
shape(...T, 'x' => int, ...U)   // multiple splats with fields
```

**Value-level splat** (`...$expr`): splat a shape value into a shape expression, analogous to Hack's existing splat for variadic arguments:

```c#
$result = shape(...$defaults, ...$overrides, 'extra' => 42);
```

**Absent fields** (`absent 'x'`): explicitly declare that a field is not present. Desugars to `?'x' => nothing` (an optional field with type `nothing`). Primarily useful on type parameter constraints to exclude specific fields from the unknown part of a shape:

```c#
function add_name<T as shape(absent 'name', mixed...)>(
  shape(...T) $s, string $name,
): shape(...T, 'name' => string) { ... }
```

## 4\. Typing

This section walks through the type system for shape splats, starting from Hack's existing shape types and building up to polymorphic shape functions and inference.

### 4.1 Representation

#### 4.1.1 Current Representation

Hack's internal representation of shape types lives in `typing_defs_core.mli`.

**Shape field keys** identify fields by string literal or class constant:

```kotlin
type tshape_field_name =
  | TSFregex_group of pos_string
  | TSFlit_str of pos_byte_string
  | TSFclass_const of pos_id * pos_string

module TShapeField : sig
  type t = tshape_field_name
  val compare : t -> t -> int
  val equal : t -> t -> bool
  ...
end

module TShapeMap : sig
  include WrappedMap.S with type key = TShapeField.t
  ...
end
```

**Shape field descriptors** pair an optional flag with a type:

```rust
and 'phase shape_field_type = {
  sft_optional: bool;
  sft_ty: 'phase ty;
}
```

**Shape types** combine a field map with an unknown-field type:

```kotlin
and 'phase shape_type = {
  s_origin: type_origin;
  s_unknown_value: 'phase ty;
  s_fields: 'phase shape_field_type TShapeMap.t;
}

| Tshape : 'phase shape_type -> 'phase ty_
```

The AST-level representation (`aast_defs.ml`) is simpler, a boolean for open/closed and a list of fields:

```kotlin
and nast_shape_info = {
  nsi_allows_unknown_fields: bool;
  nsi_field_map: shape_field_info list;
}
```

Key observations:

* `s_unknown_value` generalizes the AST's `nsi_allows_unknown_fields` boolean: `nothing` \= closed, `mixed` \= open. We are currently adding support for denotable upper bounds for unknown fields which we assume in our examples throughout e.g. `shape('x' => int, ?'y' => bool, arraykey...)`
* `shape_field_type`, `TShapeField`, `TShapeMap`, the field descriptor and field key infrastructure are unchanged by this proposal. We reuse them as-is.
* There is no representation for splat elements, `s_fields` is a flat map from field names to field types.

#### 4.1.2 Proposed Representation

We rename the current `shape_type` to `shape_type_simple` and introduce `shape_type` as a sum:

```rust
(* Renamed from shape_type; the existing representation, unchanged *)
and 'phase shape_type_simple = {
  s_origin: type_origin;
  s_unknown_value: 'phase ty;
  s_fields: 'phase shape_field_type TShapeMap.t;
}

(* NEW: an ordered list of types with rightmost-wins semantics *)
and 'phase shape_type_splat = {
  ss_elems: 'phase ty list;
}

(* Shape types are now either simple or a splat composition *)
and 'phase shape_type =
  | Shape_simple of 'phase shape_type_simple
  | Shape_splat of 'phase shape_type_splat
```

`shape_field_type`, `TShapeField`, and `TShapeMap` are unchanged.

`Shape_simple` is exactly today's shape type under a new name. All existing shapes are `Shape_simple` and behave identically to before.

`Shape_splat` represents a shape built from multiple components with rightmost-wins semantics. Each element in `ss_elems` is a type. For example:

```c#
shape(...T, 'x' => int, ...U)
```

is represented as:

```c#
Shape_splat {
  ss_elems = [
    T;
    Shape_simple { s_fields = { 'x' => {sft_optional=false; sft_ty=int} };
                   s_unknown_value = nothing; ... };
    U;
  ]
}
```

The concrete fields `'x' => int` become a `Shape_simple` element at the appropriate position in the list. Order matters, rightmost-wins resolution is determined by position in `ss_elems`.

#### 4.1.3 Shape Algebra: Rightmost-Wins Merge

Before we can explain how the surface syntax is lowered, we need to define what it means to merge two simple shapes.

***The problem.*** When two shapes contribute the same field, what is the result? For example, in `shape(...shape('x' => int), ...shape('x' => string))`, what type does `'x'` have?

***Rightmost-wins.*** The rightmost element takes precedence, matching the value-level semantics of `shape(...$a, ...$b)`: if both `$a` and `$b` have a field `'x'`, `$b`'s value is the one in the result.

But requiredness adds a subtlety. If the right element has an *optional* field, it might not be there at runtime, in which case the left's value persists. So we need to handle four cases.

***Merging field descriptors.*** The merge of two field descriptors is a right-biased override:

* Right is `Req R`: right is definitely present. It overrides the left completely, the left's type is irrelevant.
* Right is `Opt R`: right might be absent. If absent, the left's value persists at runtime, so the result type must account for both: the type is `(L | R)`.

For requiredness: the result is required if *either* side guarantees the field is present: `Req` if either input is `Req`, `Opt` only if both are `Opt`.

| Left | Right | Result |
| :---- | :---- | :---- |
| `Req _` | `Req R` | `Req R` |
| `Req L` | `Opt R` | `Req (L | R)` |
| `Opt _` | `Req R` | `Req R` |
| `Opt L` | `Opt R` | `Opt (L | R)` |

Note that we make use of the fact that we can consider any explicitly field present on the left (resp. right) to be implicitly optional on the right (resp. left) with type `mixed` for open shapes and type `nothing` for closed shapes.

***Merging two simple shapes*****.** Given two `shape_type_simple` values `left` and `right`:

* For each field: merge the field descriptors per the table above (NB this is the correct interpretation for open shapes since every field in an open shape is implicitly optional).
* For unknown: `union(left.unknown, right.unknown)`.

Since this is a little tricksy we give examples of each case, [below](#examples).

**Defaults pattern** (`test/expect/splat/defaults_pattern.query`):

The ‘defaults pattern’ falls out naturally. If `left` has `Req default` and `right` has `Opt override`, the result is `Req (default | override)`, the left's required field guarantees presence, while the right's optional override widens the type.

Query: `shape(...shape('x' => int, 'y' => string), ...shape(?'x' => bool)) <: shape('x' => (int | bool), 'y' => string)`

```
⊢ {"y" => string, "x" => (int| bool)} <: {"y" => string, "x" => (int| bool)}
  [Refl]
result: ok
```

The left splat has `'x' => Req int` and the right has `?'x' => Opt bool`. Merging: `(Req int, Opt bool) → Req (int | bool)`. The field `'y'` is only on the left, so it passes through. Both sides normalize to the same shape.

***Algebraic structure*****.** Shape merge forms a *monoid*, not a lattice join (which would require commutativity). Following Morris and McKinna's terminology for row combination, this is a right-biased override operation:

* *Associative*: `merge(merge(A, B), C) = merge(A, merge(B, C))`. This holds because for any field, the rightmost contributor is the same regardless of grouping, and the value types compose correctly since union is associative: `(L | M) | R = L | (M | R)`. Verified by property-based tests in `test_properties.ml`.
* *Has identity*: `merge(shape(), S) = merge(S, shape()) = S`, the empty closed shape contributes nothing.
* *NOT commutative*: `merge(A, B) ≠ merge(B, A)` in general, the right side's fields override the left's, not vice versa.


##### Examples {#examples}

###### *ReqL-ReqR*

```c#
shape(...shape('x' => int), ...shape('x' => bool)
'x': Left - Req int, Right - Req bool = Req bool
unknown: Left - nothing, Right - nothing = nothing
result:
shape('x' => bool)
```

###### *ReqL-OptR*

```c#
shape(...shape('x' => int), ...shape(?'x' => bool)
'x': Left - Req int, Right - Opt bool = Req (int|bool)
unknown: Left - nothing, Right - nothing = nothing
result:
shape('x' => (int|bool))
```

###### *OptL-ReqR*

```c#
shape(...shape(?'x' => int), ...shape('x' => bool)
'x': Left - Opt int, Right - Req bool = Req bool
unknown: Left - nothing, Right - nothing = nothing
result:
shape('x' => bool)
```

###### *OptL-OptR*

```c#
shape(...shape(?'x' => int), ...shape(?'x' => bool)
'x': Left - Opt int, Right - Opt bool = Opt (int|bool)
unknown: Left - nothing, Right - nothing = nothing
result:
shape(?'x' => (int|bool))
```

###### *OpenL-OpenR*

```c#
shape(...shape('x' => int, arraykey...), shape('y' => bool, mixed...))
'x' : Left - Req int, Right - Opt mixed = Req (int | mixed) = Req mixed
'y' : Left - Opt arraykey, Right - Req bool = Req bool
unknown: Left - arraykey, Right - mixed = (arraykey | mixed) = mixed
result:
shape('x' => mixed, 'y' => bool, mixed...)
```

#### 4.1.4 Lowering from Surface Syntax

The surface syntax allows free interleaving of fields and splats:

```c#
shape(...T, 'x' => int, ?'y' => string, ...U, 'z' => bool)
```

The parser preserves the interleaved list of fields and splats in the AST. During localization (§4.1.6), consecutive fields are grouped into `Shape_simple` elements and `absent 'f'` is desugared to `?'f' => nothing`, an optional field with type `nothing`, meaning the field is definitely not present. This is primarily useful on type parameter constraints (see §2.4 for the motivating example).

**`absent` does not remove fields from concrete shapes.** Under rightmost-wins merge, `absent 'f'` means "I contribute nothing for field `'f'`"; it is `Opt nothing`, i.e., the right side's field is always absent at runtime. But if the *left* side has the field, the left's value persists:

* `(Req L, Opt nothing)` → `Req (L | nothing)` \= `Req L`; unchanged
* `(Opt L, Opt nothing)` → `Opt (L | nothing)` \= `Opt L`; unchanged

So `shape(...User, absent 'age')` where `User` has `'age' => int` (required) produces a shape that still has `'age' => int`. This is a consequence of the merge algebra: the left's contribution persists when the right contributes nothing.

**Future work: `remove` keyword.** A dedicated `remove 'f'` operator could provide hard field deletion, producing `Opt nothing` regardless of the left side's requiredness. Unlike `absent`, `remove` would not desugar to a field descriptor within the merge algebra; it would be a post-merge operation that forcibly sets the field to absent. This is deferred to a future proposal pending evidence of demand.

**Absent override** (`test/expect/splat/absent_override.query`):

Query: `shape(...shape(?'x' => int, 'y' => string), ...shape(absent 'x')) <: shape(?'x' => int, 'y' => string)`

```
⊢ {"y" => string, ?"x" => int} <: {"y" => string, ?"x" => int}
  [Refl]
result: ok
```

The left has `?'x' => Opt int` and the right has `absent 'x'` \= `?'x' => Opt nothing`. Merging: `(Opt int, Opt nothing) → Opt (int | nothing) = Opt int`. The field remains optional with type `int`. This demonstrates that declaring merging a label declared as  `absent` does not remove the field; the left's contribution persists because the right contributes nothing (see the note above on `absent` limitations).

For the example:

```c#
shape(...T, 'x' => int, ?'y' => string, ...U, 'z' => bool)
```

the parser produces:

```c#
ss_elems = [T; shape('x' => int, ?'y' => string); U; shape('z' => bool)]
```

This list is then **normalized**. Normalization reduces the element list to a canonical form by applying three rules:

1. **Drop `nothing`**: `nothing` is the identity of merge; it contributes no fields and has `unknown = nothing`. Remove it.
2. **Merge adjacent simples**: When two `Shape_simple` elements are next to each other, merge them into one using the field merge operation (§4.1.3). This is always possible because both have concrete field maps.
3. **Lift singletons**: If the entire list reduces to a single element, unwrap it,`shape(...T)` is just `T`.

**Distinguished elements.**

* `nothing` (the bottom type) is uninhabited so the resulting shape type can’t be constructed. `shape(...nothing, ...S)` \= `nothing`.
* `shape()` (the unit shape type) is the left- and right-identity of merge. `shape(...shape(), …S) = shape(...S, shape()) = S`.
* `shape(mixed...)` (the top shape type, every shape is a subtype of it) is an **annihilator for openness** \- the shape resulting from the merge is always open. The behavior here is a little subtle:
  * `shape(…shape(mixed…), …shape(’req’ => int, ?’opt’ => int)) = shape(‘req’ => int, ?’opt’ => mixed, mixed…)`
  * `shape(…shape(‘req’ => int, ?’opt’ => int), …shape(mixed…) = shape(‘req’ => mixed, ?’opt’ => mixed, mixed…)`

The algorithm processes the element list left-to-right, accumulating a "pending simple" that grows by merging whenever the next element is also a simple shape. When a non-simple element (type parameter, type alias) is encountered, the pending simple is flushed to the output and the non-simple element is appended. Any element that is not a valid splat element (e.g., `int`, `string`) is an error.

##### Examples

**All concrete, collapses to `Shape_simple`:**

```c#
shape(...shape('x' => int), ...shape('y' => string))
```

The two simples are adjacent, so they merge:

```c#
→ [shape('x' => int); shape('y' => string)]
→ [shape('x' => int, 'y' => string)]          -- merge adjacent
→ Shape_simple { 'x' => int, 'y' => string }  -- lift singleton
```

**Type parameter with fields, stays as `Shape_splat`:**

```c#
shape(...T, 'x' => int)
```

```c#
→ [T; shape('x' => int)]                      -- T breaks adjacency
→ Shape_splat { ss_elems = [T; shape('x' => int)] }
```

**Fields on both sides of a type parameter:**

```c#
shape('a' => int, ...T, 'b' => string)
```

The fields before and after `T` form separate simple shapes:

```c#
→ [shape('a' => int); T; shape('b' => string)]
→ Shape_splat { ss_elems = [shape('a' => int); T; shape('b' => string)] }
```

**Interleaved fields and splats; simple shapes cannot merge across type parameters:**

```c#
shape('x' => int, ...T1, 'y' => int, ...T2, 'z' => int)
```

Each field becomes a separate `Shape_simple` because the type parameters break adjacency:

```c#
→ [shape('x' => int); T1; shape('y' => int); T2; shape('z' => int)]
→ Shape_splat { ss_elems = [shape('x' => int); T1; shape('y' => int);
                            T2; shape('z' => int)] }
```

None of the simple shapes can merge; they are separated by `T1` and `T2`. This matters because the position of each simple relative to the type parameters determines rightmost-wins resolution: if `T1` has a field `'y'`, the concrete `'y' => int` to its right overrides it; but if `T2` also has `'y'`, then `T2`'s version overrides the concrete one.

**`shape()` dropped, adjacent simple shapes merged:**

```c#
shape(...shape(), ...shape('x' => int), ...shape('y' => string), ...T)
```

```c#
→ [shape(); shape('x' => int); shape('y' => string); T]
→ [shape('x' => int); shape('y' => string); T]           -- drop shape()
→ [shape('x' => int, 'y' => string); T]                  -- merge adjacent
→ Shape_splat { ss_elems = [shape('x' => int, 'y' => string); T] }
```

**Overlapping fields, rightmost wins:**

```c#
shape(...shape('x' => int, 'y' => bool), ...shape('x' => string))
```

Adjacent simples merge. Field `'x'` appears in both; rightmost wins:

```c#
→ [shape('x' => int, 'y' => bool); shape('x' => string)]
→ [shape('x' => string, 'y' => bool)]                     -- merge: 'x' => string wins
→ Shape_simple { 'x' => string, 'y' => bool }             -- lift singleton
```

**Error conditions.** Normalization rejects splat elements that are not valid shape-like types. A splat element must be one of:

- A `Shape_simple` (concrete shape)
- A type parameter bounded by a shape type (`T as shape(mixed...)`)
- A type alias that resolves to a shape type
- A newtype; inside the defining file this expands to its definition. Outside of its defining file it is treated in the same way as a type parameter.
- `nothing`

Any other type in splat position is ill-formed:

```c#
shape(...int, 'x' => bool)           // error: int is not a shape
shape(...vec<int>, 'x' => bool)      // error: vec<int> is not a shape
shape(...(int | string), 'x' => int) // error: union is not a shape
```

The error is reported at the type level (not the parser level) because a type alias might resolve to a non-shape type:

```c#
type NotAShape = int;
shape(...NotAShape, 'x' => bool)     // error after alias resolution
```

Normalization accumulates all errors rather than failing on the first one, so all ill-formed splat elements are reported together.

**Post-normalization invariants.** After successful normalization, a `Shape_splat` is guaranteed to have:

- No `nothing` elements
- No adjacent `Shape_simple` elements (they've been merged)
- At least two elements (singletons are lifted out)
- Every element is either a `Shape_simple`, a type parameter, or a type alias resolving to a shape

#### 4.1.5 Parameterized Type Aliases

Type aliases can have type parameters, and those parameters can appear as splat elements:

```c#
type WithTimestamp<T as shape(mixed...)> = shape(...T, 'created_at' => int);
type WithEnv<T as shape(mixed...)> = shape(...T, 'is_async' => bool, 'async_id' => string);
```

These aliases are parameterized shape compositions. When instantiated with a concrete shape, the alias expands and normalizes:

```c#
type User = shape('name' => string, 'age' => int);
type TimestampedUser = WithTimestamp<User>;
// expands to: shape(...User, 'created_at' => int)
// normalizes to: shape('name' => string, 'age' => int, 'created_at' => int)
```

When the type parameter is left abstract (e.g., in a function signature), the alias expands but normalization leaves the type parameter in place:

```c#
function stamp<T as shape(mixed...)>(shape(...T) $s): WithTimestamp<T> {
  return shape(...$s, 'created_at' => time());
}
// WithTimestamp<T> expands to shape(...T, 'created_at' => int)
// T is abstract, so it remains as a splat element
```

Aliases can also compose other aliases:

```c#
type FullContext<T> = WithEnv<WithTimestamp<T>>;
// expands to: shape(...shape(...T, 'created_at' => int), 'is_async' => bool, 'async_id' => string)
// normalizes to: shape(...T, 'created_at' => int, 'is_async' => bool, 'async_id' => string)
```

The inner `WithTimestamp<T>` expands first, producing a `Shape_splat`. When this appears as a splat element in the outer alias, normalization merges the concrete fields from both aliases while preserving `T`.

**Variance.** A type parameter in splat position is **covariant**. Shape fields are covariant in their value types, and a splat element contributes fields, so a larger `T` (more fields, wider types) produces a larger shape. In `type WithTimestamp<T> = shape(...T, 'created_at' => int)`, `T` is covariant. Hack's existing variance checker infers variance for type alias parameters from their usage; splat position should be treated as covariant usage, consistent with how field types are already handled.

#### 4.1.6 When Does Normalization Happen?

Hack's type checker has two phases for types: the *declaration phase* (`decl_ty`) and the *localized phase* (`locl_ty`). The boundary between them is **localization** (`typing_phase.ml`), which expands type aliases, substitutes type parameters, and transforms declaration types into the types used during type checking.

Today, localization eagerly expands type aliases. For example, `Tapply("WithTimestamp", [User])` is expanded by substituting `User` for `T` in the alias body, producing a concrete `Tshape`. The `s_origin` field on the resulting shape records which alias it came from (for error messages and IDE features).

With shape splats, localization is the natural place to perform normalization:

1. **Alias expansion**: `WithTimestamp<User>` expands to `shape(...User, 'created_at' => int)`, producing a `Shape_splat` in declaration form.
2. **Localization**: the splat elements are localized (type parameters substituted, nested aliases expanded).
3. **Normalization**: the localized splat list is normalized, merging adjacent simples, dropping `nothing`, lifting singletons.

When all type parameters are instantiated with concrete types, normalization produces a `Shape_simple`, and the result is indistinguishable from today's shape types. When type parameters remain abstract, normalization produces a `Shape_splat` with the parameters as elements.

This means normalization runs once per localization, not repeatedly during subtyping. The subtyping algorithm (§4.4) receives already-normalized shapes. The additional "normalization for inference" step (§4.5.1) handles the further reduction needed when flex vars are introduced during inference.

#### 4.1.7 Warnings for Overlapping Fields

When two concrete splat elements contribute the same field, rightmost-wins silently overrides the left's field. This is the intended semantics for patterns like defaults:

```c#
shape('page_size' => 10, 'timeout' => 30, ...$user_opts)
```

But for type alias composition, silent overriding is often a bug:

```c#
type A = shape('x' => int, 'y' => string);
type B = shape('y' => bool, 'z' => float);
type C = shape(...A, ...B);
// 'y' silently overridden: A's string replaced by B's bool
```

The programmer likely intended `A` and `B` to have disjoint fields. We propose a **warning** when concrete splat elements have overlapping fields at the type alias level. This catches accidental overrides without making the feature harder to use for intentional patterns.

The warning fires during normalization when merging two `Shape_simple` elements that share a field name. It does not fire for:

- Fields overridden by explicit field syntax (e.g., `shape(...A, 'y' => bool)`), as that's clearly intentional.
- Type parameter splats, since their fields are unknown at definition time.

---

### 4.2 Expression-Level Shape Splats

The previous section covered shape splats at the type level, how `shape(...T, 'x' => int)` is represented and normalized as a type. This section covers shape splats at the expression level, how `shape(...$a, 'x' => 1)` is typed.

#### 4.2.1 Typing Shape Expressions with Splats

Today, a shape expression `shape('x' => 1, 'y' => 'hello')` has a straightforward typing rule: each field's type is inferred from its value, producing a closed simple shape.

With splats, a shape expression can include `...$expr` elements:

```c#
$a = shape('x' => 1, 'y' => true);
$b = shape(...$a, 'z' => 'hello');
// $b : shape('x' => int, 'y' => bool, 'z' => string)
```

The typing rule for a shape expression with splats:

1. Type each `...$expr` element. Its type must be a shape (otherwise error).
2. Type each `'key' => $value` element to get its field descriptor.
3. Group the elements into a `Shape_splat` element list (same representation as the type-level splat, §4.1.4).
4. Normalize the element list. If all elements are concrete, this produces a `Shape_simple`, the fully resolved type.

The result type of the shape expression is the normalized shape type.

**Nested splats.** A splat expression can itself be a shape expression with splats:

```c#
$result = shape(...shape(...$a, 'x' => 1), 'y' => 2);
```

This works naturally: the inner `shape(...$a, 'x' => 1)` is typed as a shape expression (step 1 above requires it to be a shape), and its result type is splatted into the outer expression. At the type level, normalization (§4.1.4) flattens the nesting: if `$a` has type `shape('z' => bool)`, the inner expression has type `shape('z' => bool, 'x' => int)`, and the outer normalizes to `shape('z' => bool, 'x' => int, 'y' => int)`. At runtime, the inner expression evaluates to a shape value which is then merged into the outer shape (§4.2.5).

#### 4.2.2 Rightmost-Wins at the Value Level

Rightmost-wins semantics at the value level match the type level. When two splat elements contribute the same field, the rightmost value appears in the result:

```c#
$defaults = shape('page_size' => 10, 'timeout' => 30);
$overrides = shape('timeout' => 60);
$config = shape(...$defaults, ...$overrides);
// $config : shape('page_size' => int, 'timeout' => int)
// $config['timeout'] === 60  (rightmost wins)
```

This is analogous to JavaScript's `{...defaults, ...overrides}`.

#### 4.2.3 Relationship to `Shape\fb\merge`

Hack already has an n-ary shape merge function, `Shape\fb\merge`:

```c#
namespace Shape\fb;

function merge<T as shape(...)>(T $first, T ...$rest)[]: T {
  $as_array = Shapes::toArray($first);
  foreach ($rest as $r) {
    foreach (Shapes::toArray($r) as $k => $v) {
      $as_array[$k] = $v;
    }
  }
  return \HH\FIXME\UNSAFE_CAST<dict<arraykey, mixed>, T>(
    $as_array,
    'FIXME[4110] We\'re lying to Hack here ...',
  );
}
```

The runtime semantics are exactly rightmost-wins: iterate through each shape left-to-right, overriding fields from earlier shapes. Our expression-level splat `shape(...$a, ...$b, ...$c)` has the same runtime behavior.

However, the *typing* of `Shape\fb\merge` is imprecise. All arguments and the result share the same type parameter `T`, so Hack unifies the input types, losing information:

```c#
function merge_closed(
  shape('x' => int, 'y' => bool) $has_x_y,
  shape('y' => int, 'z' => bool) $has_y_z,
): mixed {
  $out = Shape\fb\merge($has_x_y, $has_y_z);
  // T unifies to: shape(?'x' => int, 'y' => (int | bool), ?'z' => bool)
  return $out;
}
```

The result type is safe (it's a supertype of the actual value) but imprecise in two ways:

1. **Requiredness is lost.** `'x'` is definitely present (from `$has_x_y`) and `'z'` is definitely present (from `$has_y_z`), but both become optional in the unified `T` because neither appears in both inputs.
2. **Field types are widened.** `'y'` has type `int | bool` even though at runtime rightmost-wins means it's always `int` (from `$has_y_z`).

With open shapes, the imprecision is worse:

```c#
function merge_open(
  shape('x' => int, 'y' => bool, ...) $has_x_y_open,
  shape('y' => int, 'z' => bool, ...) $has_y_z_open,
): mixed {
  $out = Shape\fb\merge($has_x_y_open, $has_y_z_open);
  // solves to: shape(?'x' => mixed, 'y' => (int | bool), ?'z' => mixed, ...)
  return $out;
}
```

Non-common fields become `?'x' => mixed` because the open shape could have any field with any type.

**With expression-level splats**, each operand keeps its own type, and the merge operation (§4.1.3) computes the result precisely:

```c#
$merged_closed = shape(...$has_x_y, ...$has_y_z);
// shape('x' => int, 'y' => int, 'z' => bool)
//   'x'     => Left: Req int,      Right: Opt nothing = Req int
//   'y'     => Left: Req bool,     Right: Req int     = Req int
//   'z'     => Left: Opt nothing,  Right: Req bool    = Req bool
//   unknown => Left: nothiong,     Right: nothing     = nothing

$merged_open = shape(...$has_x_y_open, ...$has_y_z_open);
// shape('x' => mixed, 'y' => int, 'z' => bool, ...)
//   'x'     => Left: Req int,   Right: Opt mixed = Req (int|mixed) = Req mixed
//   'y'     => Left: Req bool,  Right: Req int                     = Req int
//   'z'     => Left: Opt mixed, Right: Req bool                    = Req bool
//   unknown => Left: mixed,     Right: mixed                       = mixed
```

**N-ary merge.** `Shape\fb\merge` is n-ary (`T $first, T ...$rest`), taking any number of shapes. Expression-level splats handle this naturally through multiple `...` elements:

```c#
// Shape\fb\merge($a, $b, $c)  becomes:
$result = shape(...$a, ...$b, ...$c);
```

The type of the result is determined by normalizing the element list `[$a_type; $b_type; $c_type]`. Adjacent concrete shapes merge via the merge operation (§4.1.3), and rightmost-wins applies across all elements. Each operand keeps its own type, and the result type reflects the actual field composition.

#### 4.2.4 Polymorphic Shape Expressions

When the type of a shape expression contains splat type parameters (because the splat expression has a polymorphic type), the result type preserves those parameters:

```c#
function with_timestamp<T as shape(mixed...)>(
  shape(...T) $s,
): shape(...T, 'created_at' => int) {
  return shape(...$s, 'created_at' => time());
}
```

The expression `shape(...$s, 'created_at' => time())` is typed as follows:

1. `$s` has type `shape(...T)`, which normalizes to just `T`.
2. `time()` has type `int`.
3. The element list is `[T; shape('created_at' => int)]`.
4. After normalization: `Shape_splat { ss_elems = [T; shape('created_at' => int)] }`.
5. The result type is `shape(...T, 'created_at' => int)`, matching the declared return type.

This is the key mechanism that makes polymorphic shape functions work at the expression level: the splat expression preserves the type parameter `T` from the input, and the concrete field `'created_at'` is added alongside it.

#### 4.2.5 Runtime Semantics

At runtime, `shape(...$a, 'x' => 1, ...$b)` evaluates left-to-right through the interleaved list of elements:

1. Start with an empty dict.
2. For each element in order:
   - `...$expr`: merge all fields from `$expr` into the dict.
   - `'key' => $value`: set the field in the dict.
3. Rightmost-wins: later elements override earlier ones for the same key.

The order matters. In `shape('x' => 1, ...$a, 'x' => 2)`, the final `'x' => 2` overrides both the initial `'x' => 1` and any `'x'` from `$a`.

This is the same semantics as `Dict\merge` but without leaving the shape type system. The type checker statically verifies that the result has the expected shape type, so no runtime type assertion (`TypeAssert::coerce`) is needed.

---

### 4.3 Shape Operations

Shape splats introduce a new type representation (`Shape_splat` alongside `Shape_simple`). This section analyses how existing shape typing operations interact with the new variant.

When all type parameters are instantiated with concrete shapes, normalization (§4.1.4) reduces the splat to a `Shape_simple`, and all existing operations see a normal shape. The interesting case is when type parameters are abstract (e.g., inside a polymorphic function body), where the type `shape(...T, 'x' => int)` remains a `Shape_splat`.

Today, code that handles `Tshape` pattern-matches on `shape_type` and accesses `s_fields` directly. With the new sum type, each such site must handle both `Shape_simple` (unchanged) and `Shape_splat` (new). The key question is whether the *semantics* change, not just the dispatch. As we show below, the semantics are unchanged in every case.

#### 4.3.1 Operations That Work Transparently

Several `Shapes::` built-in functions work through **coercion**, not field map inspection. They construct a "fake" super-shape type with the expected field and coerce the input against it via subtyping. This means they never need to destructure the input shape's field map, so `Shape_splat` types are handled transparently.

**`Shapes::idx` and `Shapes::at`** (`typing_shapes.ml`). When called with a literal field name, the type checker transforms the generic HHI function signature into a field-specific one. For example, `Shapes::idx($s, 'name')` transforms the expected shape parameter type to:

```kotlin
MakeType.open_shape r
  ~kind:(MakeType.mixed r)
  (TShapeMap.singleton field_name { sft_optional = true; sft_ty = field_ty })
```

This is an open shape requiring only the accessed field. The input `$s` is coerced against this shape via `Typing_coercion.coerce_type`. Whether `$s` is a `Shape_simple` or `Shape_splat`, the coercion delegates to subtyping, which handles both.

**`Shapes::keyExists`** (`typing_shapes.ml`). Refines the shape type by intersecting with a constructed shape. For `Shapes::keyExists($s, 'name')`, the refined type is the intersection of `$s`'s type with `shape(?'name' => mixed, mixed...)`. This works through intersection/subtyping, not field map destructuring.

**Field access** (`$s['key']`). For reading, the type checker uses `widen_for_array_get_ci` (`typing_array_access_util.ml`) to construct a tighter constraint, then falls back to normal subtyping. Today, `widen_for_array_get_ci` pattern-matches on `Tshape` and accesses `s_fields`. For `Shape_splat`, the function walks `ss_elems` right-to-left (same lookup as field assignment, §4.3.2): if the field is found in a concrete `Shape_simple` element, it returns the tightened constraint from that element's field type. If the field is only in an abstract element (type parameter), it returns `None`, falling back to normal subtyping via the parameter's bound. Semantics are unchanged; precision is preserved for fields in the concrete part of the splat.

#### 4.3.2 Operations That Modify the Field Map

Some operations pattern-match on `Tshape`, destructure `s_fields`, and construct a new `Tshape` with a modified field map. These need consideration for `Shape_splat`.

**Field assignment** (`$s['key'] = val`, `typing_array_access.ml`). The assignment path goes through `GenericRules.apply_rules`, which peels away unions, intersections, newtypes, and generics before dispatching to the callback. For `Tgeneric`, it resolves to upper bounds:

```c#
(* typing_generic_rules.ml *)
| (r, Tgeneric _) ->
  let (env, tyl) = get_transitive_upper_bounds env ty in
  ...
  iter ~supportdyn ~is_nonnull env ty
```

Everything else (including `Tshape`) falls through to the callback, which pattern-matches on the shape and modifies the field map:

```kotlin
(* typing_array_access.ml *)
| Tshape { s_fields = fdm; ... } ->
  let fdm' = TShapeMap.add field { sft_optional = false; sft_ty = ty2 } fdm in
  mk (r, Tshape { s_origin = Missing_origin;
                   s_unknown_value = shape_kind; s_fields = fdm' })
```

With `Shape_splat`, this `Tshape` match needs a new case. For a `Shape_splat` like `shape(...T, 'x' => int)`, field assignment would resolve through the splat's concrete parts and T's bound, producing a widened type, the same behaviour as for `Tgeneric` today.

This widening is why the following function cannot be typed today. Consider:

```c#
function update_user_open<
  T as shape('id' => arraykey, 'name' => string, 'age' => int, ...),
>(T $input): T {
  $input['id'] = $input['id'].'!';
  $input['name'] = $input['name'].'!';
  $input['age'] = $input['age'] + 1;
  // Error: shape('id' => string, 'age' => int, 'name' => string, ...) </: T
  return $input;
}
```

The return type is `T`, but after the assignments the type checker has widened `$input` to `shape('id' => string, 'name' => string, 'age' => int, ...)`, the upper bound of `T`. The identity of `T` is lost.

This widening is correct in general: `T`'s `'id'` field could be more specific than `arraykey` (e.g., `int`), and after assigning `$input['id'].'!'` (type `string`), the field genuinely has type `string`, not whatever `T` originally had.

**With shape splats**, the same function becomes typeable:

```c#
function update_user_poly<
  T as shape(mixed...),
>(shape('name' => string, 'age' => int, ...T) $input):
  shape('name' => string, 'age' => int, ...T) {
  $input['name'] = $input['name'].'!';
  $input['age'] = $input['age'] + 1;
  return $input;
}
```

The key difference: `'name'` and `'age'` are in the **concrete part** of the shape, not in `T`. The `Shape_splat` match case for field assignment works as follows:

1. **Look up the field in the concrete parts.** Walk the `ss_elems` list right-to-left (rightmost-wins). If a `Shape_simple` element has the field, that element is authoritative.
2. **If found in a concrete element:** Modify the field in that `Shape_simple`'s field map (same as today's `Shape_simple` path). The rest of the splat — including `T` — is unchanged. The result type is the same `Shape_splat` with the updated concrete element.
3. **If not found in any concrete element:** The field must come from an abstract element (`T`). This case falls through to the existing `GenericRules` path, which resolves `T` to its upper bound and widens. The identity of `T` is lost, same as for `Tgeneric` today.

For `$input['name'] = $input['name'].'!'` on `shape(...T, 'name' => string)`:

- Step 1: `'name'` is found in the concrete `Shape_simple` element (rightmost, authoritative).
- Step 2: The field is updated in the concrete element. `T` is untouched. The result type is `shape(...T, 'name' => string)` — unchanged, since the assigned value (`string . string`) has type `string`, matching the existing field type.

The critical property: fields in the concrete part can be modified without losing `T`'s identity, because the concrete part and abstract part are structurally separated in the `Shape_splat` representation.

**`Shapes::removeKey`** (`typing_shapes.ml`). The `remove_key` function goes through `refine_handle_unions_dyn`, which pattern-matches on `Tshape` and destructures `s_unknown_value` and `s_fields` (line 220). For `Shape_simple`, it either deletes the field (closed shape) or marks it as `?'key' => nothing` (open shape). For `Shape_splat`, this match would need a new case. The semantics for a splat shape: removing a field that's in the concrete part works the same way. Removing a field that might be in the abstract part (T) is equivalent to overriding it with `absent`: `shape(...T, absent 'key')`.

**`Shapes::toDict` / `Shapes::toArray`** (`typing_shapes.ml`). The `to_collection` function pattern-matches on `Tshape` and enumerates `TShapeMap.keys` and `TShapeMap.values` to compute a union of key and value types. It needs a new case for `Shape_splat`. For a splat shape with abstract parts, the full field set is unknown, the same situation as open shapes today. The function would return `dict<arraykey, T_unknown>` where `T_unknown` is derived from the abstract part's bound.

#### 4.3.3 Shape Literal Construction

Today, shape literals (`shape('x' => 1, 'y' => 'hello')`) are typed in `typing.ml` by building a `TShapeMap` from the literal fields, producing a `Shape_simple`. This is unchanged.

Expression-level splats (`shape(...$a, 'x' => 1)`) are a new expression form with their own typing rule, covered in §4.2. They do not modify the existing shape literal typing path.

#### 4.3.4 Summary

| Operation | Mechanism | New match case? | Semantics change? |
| :---- | :---- | :---- | :---- |
| `Shapes::idx` | Coercion | No | No |
| `Shapes::at` | Coercion | No | No |
| `Shapes::keyExists` | Intersection | No | No |
| `$s['key']` (read) | Widening \+ subtyping | Yes (R-to-L lookup) | No |
| `$s['key'] = val` (write) | Callback on `Tshape` | Yes (resolve splat) | No |
| `Shapes::removeKey` | `refine_handle_unions_dyn` | Yes (handle splat) | No |
| `Shapes::toDict` | `to_collection` | Yes (use bound) | No |
| `shape(...[expr])` literal | New expression form | N/A | New (§4.2) |

Operations that work through coercion/subtyping (`Shapes::idx`, `Shapes::at`, `Shapes::keyExists`) handle `Shape_splat` transparently with no code changes.

Operations that pattern-match on `Tshape` and access `s_fields` (`field access`, `field assignment`, `removeKey`, `toDict`) need a new match case for `Shape_splat`, but the semantics are unchanged: each case resolves the splat through its concrete parts and the abstract part's bound, producing the same result as for open shapes or generic shapes today.

**Intersection types.** Intersections of `Shape_splat` types (arising from `is`/`as` refinements, `Shapes::keyExists`, or multiple type parameter bounds) decompose through Hack's existing intersection subtyping: each component of the intersection is checked separately using the splat subtyping rules (§4.4). No special intersection-of-splats simplification is needed for correctness.

---

### 4.4 Subtyping

#### Reading the traces

The subtyping and inference rules in this section and §4.5 are illustrated with traced examples from the mousetrap prototype, a standalone OCaml implementation of the shape splat type system.

**Query syntax.** A `.query` file specifies a subtyping judgment and an expected result:

```c#
show: shape('x' => int) <: shape('x' => int, mixed...)
expect: sat
```

**Type variables.** The type system uses two kinds of type variables:

- A **rigid variable** represents a type parameter that is fixed but unknown inside a polymorphic function body. The function must be correct for every valid instantiation. Rigid variables are substituted with their bounds during subtyping (§4.4.3).
- A **type variable** (inference variable) represents a type that the solver must determine. It accumulates upper and lower bounds as subtyping constraints are solved; the inferred type is the tightest type consistent with all bounds. When a polymorphic function is *called*, each type parameter is instantiated to a type variable that inference constrains.


Types use a compact notation:

- `shape('x' => int)` \- closed shape (unknown \= `nothing`)
- `shape('x' => int, mixed...)` \- open shape (unknown \= `mixed`)
- `?'x' => int` \- optional field
- `absent 'x'` \- sugar for `?'x' => nothing`
- `...T` \- splat a type parameter
- `#[v]` \- a named type variable
- `#[v as shape(mixed...) super nothing]` \- type variable with bounds
- `forall T as shape(mixed...) super nothing. τ` \- bounded universal

**Trace syntax.** The `.query.exp` file contains the subtyping proof trace. Reading a trace:

```c#
⊢ {"x" => int} <: {"x" => int, mixed...}     -- the judgment
  [Shape]                                    -- rule applied
  ⊢ nothing <: mixed                         -- sub-obligation: unknown
    [Top]
  ⊢ int <: int                               -- sub-obligation: field 'x'
    [Refl]
result: ok                                   -- judgment holds
```

- `⊢ τ₁ <: τ₂` is a subtyping judgment.
- `[RuleName]` names the rule that discharged it.
- Indentation shows the proof tree structure.
- `bind flex #[v]` introduces a type variable.
- `observe #[v] ≥ τ` records a lower bound on a type variable.
- `observe #[v] ≤ τ` records an upper bound on a type variable.
- The `flex:` section at the end shows inferred bounds.
- `result: ok` means the judgment holds; `result: error (...)` means it fails, with the reason.

#### 4.4.1 Simple Shape Subtyping

The existing subtyping rule for simple shapes, unchanged by this proposal. We give it a name for reference.

##### **\[Shape\]**

```c#
  u₁ <: u₂
  ∀ f ∈ dom(F₁) ∪ dom(F₂) :  proj(F₁, f, u₁) <:ᶠ proj(F₂, f, u₂)
  ────────────────────────────────────
                 shape(F₁, u₁) <: shape(F₂, u₂)
```

where `proj(F, f, u) = F(f)` if `f ∈ dom(F)`, else `Opt(u)`.

The first premise checks that the unknown-field types are subtypes. The second checks every field in either shape pointwise, projecting missing fields as `Opt(u),`where `u` is the type of unknown fields.

Field descriptor subtyping `<:ᶠ` is defined by four rules, `Field-Req`, `Field-Opt` , `Field-Req-Opt` and `Field-Opt-Req`

##### **\[Field-Req\]**

```c#
      τ₁ <: τ₂
  ──────────
  Req τ₁ <:ᶠ Req τ₂
```

##### **\[Field-Opt\]**

```c#
      τ₁ <: τ₂
  ──────────
  Opt τ₁ <:ᶠ Opt τ₂
```

##### **\[Field-Req-Opt\]**

```c#
      τ₁ <: τ₂
  ──────────
  Req τ₁ <:ᶠ Opt τ₂
```

##### **\[Field-Opt-Req, always fails\]**

```c#
  Opt τ₁ </:ᶠ Req τ₂
```

An optional field never satisfies a required field, you can't pass "maybe absent" where "definitely present" is needed.

Recall that `absent 'f'` desugars to `Opt nothing`. Since `nothing <: τ` for all `τ`, an absent field is a subtype of any optional field: `Opt nothing <:ᶠ Opt τ`.

##### Traces

###### ***Reflexivity** :*

Query: `shape('x' => int) <: shape('x' => int)`

```c#
⊢ {"x" => int} <: {"x" => int}
  [Refl]
result: ok
```

###### ***Closed \<: open** :*

Query: `shape('x' => int) <: shape('x' => int, mixed...)`

```c#
⊢ {"x" => int} <: {"x" => int, mixed...}
  [Shape]
  ⊢ nothing <: mixed       -- unknown: closed <: open
    [Top]
  ⊢ int <: int       -- field 'x'
    [Refl]
result: ok
```

The first sub-obligation `nothing <: mixed` checks the unknown types. A closed shape (`unknown = nothing`) is always a subtype of an open shape (`unknown = mixed`) with the same fields.

###### ***Open \</: closed** :*

Query: `shape('x' => int, mixed...) <: shape('x' => int)`

```c#
result: error (mixed is not a subtype of nothing)
```

The reverse does not hold: an open shape is not a subtype of a closed shape. The open shape might have extra fields that the closed shape forbids.

###### ***Absent \<: optional** :*

Query: `shape(absent 'x') <: shape(?'x' => int)`

```c#
⊢ {?"x" => nothing} <: {?"x" => int}
  [Shape]
  ⊢ nothing <: nothing       -- unknown
    [Bot]
  ⊢ nothing <: int       -- field 'x': Opt nothing <: Opt int
    [Bot]
result: ok
```

###### ***Extra required field fails** :*

Query: `shape('x' => int, 'y' => bool) <: shape('x' => int)`

```c#
result: error (bool is not a subtype of nothing)
```

The sub has `'y' => Req bool` but the super (closed, `unknown = nothing`) projects `'y'` as `Opt nothing`. `Req bool <:ᶠ Opt nothing` requires `bool <: nothing`, which fails. A closed super rejects extra required fields.

#### 4.4.2 Concrete Splat Subtyping

When both sides are fully resolved (no type parameters or inference variables), splat subtyping reduces to **\[Shape\]**.

For example, given the following shape splats normalization will always recover a simple shape

```c#
shape( ...shape('x' => int, 'y' => bool), ...nothing)
↓ shape('x' => int, 'y'=> bool)


shape( ...shape('x' => int, 'y' => bool), ...shape(mixed...))
↓ shape('x' => mixed, 'y'=> mixed, mixed...)


shape( ...shape(), ...shape('x' => int, 'y' => bool))
↓ shape('x' => int, 'y'=> bool)

shape( ...shape(), ...shape('x' => int, 'y' => bool), ...shape(absent 'x', absent 'y', mixed...))
↓ shape('x' => int, 'y'=> bool, mixed...)
```

##### **\[Splat-Concrete\]**

This rule just serves to bring normalization into the typing rules:

```c#
  normalize(S₁) = s₁     normalize(S₂) = s₂     s₁ <: s₂
  ─────────────────────────────────
                        S₁ <: S₂
```

Both sides normalize to `Shape_simple` (all splat elements are concrete shapes that merge together), then the simple shape subtyping rule **\[Shape\]** applies.

##### Trace

###### ***Concrete splat normalizes to reflexivity** :*

Query: `shape('x' => int, 'y' => string) <: shape(...shape('x' => int), ...shape('y' => string))`

```c#
⊢ {"y" => string, "x" => int} <: {"y" => string, "x" => int}
  [Refl]
result: ok
```

The right side's two adjacent simples merge into one, and the judgment reduces to reflexivity.

#### 4.4.3 Type Parameters in Splats

When a shape contains type parameters that represent a fixed but unknown type where the function must be correct for any valid instantiation, normalization cannot fully resolve the shape. The type parameters remain as splat elements, for example:

```c#

shape( ...shape('x' => int, 'y' => bool), ...T, ...shape(mixed...))
↓ shape( ...shape('x' => int, 'y' => bool), ...T, ...shape(mixed...))

shape( ...shape(), ...T, ...shape('x' => int, 'y' => bool))
↓ shape(...T, shape('x' => int, 'y'=> bool))

shape( ...shape(), ...shape('x' => int, 'y' => bool), ...T, ...shape(absent 'x', absent 'y', mixed...))
↓ shape(...shape('x' => int, 'y'=> bool), ...T, ...shape(absent 'x', absent 'y', mixed...))
```

For subtyping, type parameters are substituted with their bounds before normalization:

##### **\[Splat-Rigid-Sub\]**

```c#
  S₁[R ↦ upper(T)] <: S₂
  ──────────────     (Type parameter T appears in subtype)
        S₁ <: S₂
```

##### **\[Splat-Rigid-Super\]**

```c#
  S₁ <: S₂[R ↦ lower(T)]
  ─────────────     (Type parameter T appears in supertype)
        S₁ <: S₂
```

In subtype position, substitute with the upper bound. This makes `T`'s contribution as large as possible, which makes the subtype the hardest to prove `<: super`. If the judgment holds with the upper bound, it holds for all valid instantiations of `T` (which are subtypes of the upper bound). Shape fields are covariant, so a larger `T` in splat position produces a larger shape.

In supertype position, substitute with the lower bound. This makes the super as small as possible, again the hardest case. If the judgment holds with the smallest super, it holds for all valid instantiations.

After substitution, the shape may normalize further (the bound might be a concrete shape that merges with adjacent simples). If the result is fully concrete, it reduces to **\[Splat-Concrete\]**.

The motivation for why type parameters in splats matter, including a detailed example of shape field assignment on abstract types and how splats solve the problem, is in §4.3.2.

##### Note: does this break compositionality?

This does not compose with the existing subtyping rule for type parameters. Operationally, we need this rule since without it we can decompose the subtype proposition. Conceptually, I think this is happening because we are actually describing row subtyping here and  type parameters used as shape splats are actually row parameters. The subtyping rules are non-compositional because they are operating on different kinds.

---

### 4.5 Inference

**How polymorphic shape functions connect to inference.** When a polymorphic shape function is called, Hack's standard quantifier handling creates a flexible type variable for each type parameter. For example:

```c#
function add_timestamp<T as shape(mixed...)>(T $shp): shape(...T, 'created_at' => int) { ... }


add_timestamp(shape('name' => 'Bob', 'age' => 30));
```

produces the following subtyping propositions:

```c#
forall (T as shape(mixed...)). (function(shape(...T)): shape(...T, 'created_at' => int))
    <: (function(shape('name' => string, 'age' => int)): #1)

[INST-FORALL-L]

(function(shape(...#[T])): shape(...#[T], 'created_at' => int))
    <: (function(shape('name' => string, 'age' => int)): #1)

[FUN]

shape('name' => string, 'age' => int) <: shape(...#[T])
    /\ shape(...#[T], 'created_at' => int) <: #1

```

where `#[T]` is a type variable introduced when instantiating the type parameter, `T`, of the polymorphic function and `#1` is a fresh type variable introduced when typing the function call expression. The inference rules must accumulate upper and lower bounds on each type variable so that, given the variance at which the originating type parameter occured, we can solve for a type.

#### 4.5.1 The Trick

The key challenge in inference arises because, in the general case, there is no obvious way to decompose subtype propositions where one or both shape types contain one or more type variables. The ‘trick’ to doing this is to introduce fresh type variables for fields which *may* be present the type variable splat.

Given the shape polymorphic function `merge_strip` and the function call in `call`:

```c#
function merge_strip<
  T1 as shape(mixed...),
  T2 as shape(mixed...)
>(
  shape('x' => int, ...T1) $has_x,
  shape('y' => int, ...T2) $has_y,
): shape(...T1, ...T2) {  etc }

function call(
  shape('x' => int, 'y' => bool, 'z' => int) $has_x,
  shape('y' => int, ?'z' => bool) $has_y
): void {
  $result = merge_strip($has_x, $has_y);
}
```

We end up with the following subtype propositions

```c#
∀ (T1 ≤ shape(mixed...)) (T1 ≤ shape(mixed...)). (function(shape('x' => int, ...T1), shape('y' => int, ...T2)): shape(...T1, ...T2))
<: (function(shape('x' => int, 'y' => bool, 'z' => int), shape('y' => int, ?'z' => bool)): #1)

[INST-FORALL-L]

(function(shape(...shape('x' => int), ...#[T1]), shape(...shape('y' => int), ...#[T2])): shape(...#[T1], ...#[T2]))
<: (function(shape('x' => int, 'y' => bool, 'z' => int), shape('y' => int, ?'z' => bool)): #1)

[FUN]

shape('x' => int, 'y' => bool, 'z' => int) <: shape(...shape('x' => int), ...#[T1])[1] ⋀
shape('y' => int, ?'z' => bool) <: shape(...shape('y' => int), ...#[T2])           [2] ⋀
#1 <: shape(...#[T1], ...#[T2])                                                    [3]


inference env:
{ nothing ≤ #[T1] ≤ shape(mixed...)
, nothing ≤ #[T2] ≤ shape(mixed...)
, nothing ≤ #1 ≤ shape(...#[T1], ...#[T2])
}
```

Looking at the concrete types involved at the callsite, it’s clear that we should end up with the following solutions for our three type variables:

```c#
#1 := shape('y' => bool, 'z' => (int|bool), absent 'x')
#[T1] := shape('y' => bool, 'z' => int, absent 'x')
#[T2] := shape(?'z' => bool, absent 'y')
```

The question is, how do we get there?

Let’s focus on  proposition `[1]`:

```c#
shape('x' => int, 'y' => bool, 'z' => int) <: shape(...shape('x' => int), ...#[T1])
```

For this to be true, several conditions need to be met:

1) Since neither `y` or `z` fields are present in the concrete part of the supertype, they *must* be contained in the solution to `#[T1]`
2) `#[T1]` appears rightmost in the supertype and we can’t prove it doesn’t contain `x` (`x` wasn’t `absent` in the underlying type parameter bounds) so it *may* contain a field `x` but we don’t know at what type
3) Under rightmost-wins semantics, the type of `x` in the merged shape will be the union of `int` and whatever type `x` has in `#[T1]`

These conditions let us decompose `[1]`into two finer grained proposition:

```c#
shape('y' => bool, 'z' => int, ?'x' => #[x]) <: #[T1] [1.a]   ⋀
int <: (int | #[x])                                   [1.b]

inference env:
{ shape('y' => bool, 'z' => int, ?'x' => #[x]) ≤ #[T1] ≤ shape(mixed...) : covariant
, nothing ≤ #[T2] ≤ shape(mixed...)
, nothing ≤ #1 ≤ mixed
, nothing ≤ #[x] ≤ mixed
}
```

Where the fresh type variable `#[x]` was introduced as the as-yet-unknown type of the optional field `x` in `#[T1]`.

The first new proposition, `[1.a],` gives us the correct lower bound and the observation that the type parameter `T1` occurs only covariantly.  The second proposition,`[1.a]` , is handled using the existing subtyping rules for unions; as a consequence it will be satisfied by `int <: int` , the type variable `#[x]` will receive no further constraints and will be observed only in a covariant position. As a consequence it will be solved to `nothing`.

Since `#[x]` is solved to `nothing`, and `#[T1]` occurs only contravariantly, it will be solved to `shape('y' => bool, 'z' => int, ?'x' => #[x])` or more simply,  `shape('y' => bool, 'z' => int, absent ‘x’)` which is the answer we were looking for\! Note that the `absent ‘x’` was *inferred* i.e. we didn’t have to write this in the bounds of `T1` ,  though doing so would have avoided the second proposition.

Proposition `[2]` follows a very similar pattern, ultimately solving to `shape(?'z' => bool, absent 'y')`.

Finally, for `#1`, the fresh type variable introduced for the return type of the function expression, we now have solutions to both `#[T1]` and `#[T2]` so we can substitute and normalize the shape splat:

```c#
nothing ≤ #1 ≤ shape(...#[T1], ...#[T2])

[substitute]

nothing ≤ #1 ≤ shape(...shape('y' => bool, 'z' => int, ?‘x’ => nothing ) , ...shape(?'z' => bool, ?'y' => nothing))

[normalize]

nothing ≤ #1 ≤ shape('y' => bool, 'z' => (int|bool), ?‘x’ => nothing)
```

Since `#1` was used only in a contravariant position it is solved to its upper bounds; using our syntactic sugar this gives us our expected return type:

```c#
shape('y' => bool, 'z' => (int|bool), absent 'x')
```

A full description of all inference cases is given in the [Inference cases tab].

#### 4.5.2 Underdetermined propositions and Well-formedness

##### **Where underdetermined propositions come from**

All the shape subtyping cases described in the previous section involve a single type variable on each side. But what happens when a splat contains multiple type variables?

Consider a function that takes a shape composed from two type parameters:

```c#
function process<T1 as shape(mixed...), T2 as shape(mixed...)>(
  shape('flag' => bool, ...T1, ...T2) $data
): void { ... }

process(shape('flag' => true, 'x' => int, 'y' => string));
```

```c#
∀ (T1 ≤ shape(mixed...)) (T2 ≤ shape(mixed...)).
  (function(shape('flag' => bool, ...T1, ...T2)): void)
  <: (function(shape('flag' => true, 'x' => int, 'y' => string)): #1)

[INST-FORALL-L]

(function(shape(...shape('flag' => bool), ...#[T1], ...#[T2])): void)
  <: (function(shape('flag' => true, 'x' => int, 'y' => string)): #1)

[FUN]

shape('flag' => true, 'x' => int, 'y' => string)
<: shape(...shape('flag' => bool), ...#[T1], ...#[T2])   [1]
```

Proposition `[1]` has two type variables in the supertype. After discharging `'flag'` against the concrete part, we're left with fields `'x'` and `'y'` that must be split between `#[T1]` and `#[T2]`. But how? Should `'x'` go to `T1` and `'y'` to `T2`? Vice versa? Both to one? For `n` fields there are `2^n` possible partitions, and we have no basis to choose.

##### **What we do**

The normalization step handles this by reducing multiple type variables down to one. It keeps the rightmost type variable and replaces all others with their current lower bound from the inference environment. In the worst case (type variable not yet constrained), the lower bound is `nothing` ; in this case, we use the unit shape-type, `shape()`, which gets dropped from splats during normalization so the replaced type variable simply vanishes.  Note: `shape()` is not the bottom shape (`shape() </:  shape('x' => int)`) but it *is* the element that is least informative. It makes `merge(shape(), X) = X`, which is what matters here: "I don't know what this type variable is, so I'll assume it contributes nothing."

So the supertype `shape(...shape('flag' => bool), ...#[T1], ...#[T2])` normalizes as follows: `#[T1]` is replaced by its lower bound `nothing`, which vanishes. The concrete parts merge: `merge(shape('flag' => bool), shape()) = shape('flag' => bool)`. After normalization, the concrete part is `shape('flag' => bool)` with `#[T2]` rightmost.

Now the check proceeds as the standard concrete `<:` type-variable-rightmost case using the fresh-variable technique. All non-flag fields end up in `#[T2]`'s lower bound. But `#[T1]` gets **no constraints at all** from this proposition; it was replaced and removed.

```c#
inference env:
{ nothing ≤ #[T1] ≤ shape(mixed...)       [unconstrained, no bounds accumulated]
, shape(?'flag' => #f, 'x' => int, 'y' => string) ≤ #[T2] ≤ shape(mixed...)
}
```

The proposition succeeds, but `#[T1]` is completely undetermined; it receives no constraints from this check.

##### **When underdetermined propositions are fine**

An undetermined proposition is fine as long as the type variable receives constraints from another occurrence. Consider a function where each type parameter appears both in a multi-splat (undetermined) and as the sole type variable in its own parameter (determined):

```c#
function merge<T1 as shape(mixed...), T2 as shape(mixed...)>(
  shape(...T1, 'x' => int) $a,
  shape(...T2, 'y' => int) $b,
): shape(...T1, ...T2) { ... }

merge(shape('a' => bool, 'x' => int), shape('b' => string, 'y' => int));
```

```c#
∀ (T1 ≤ shape(mixed...)) (T2 ≤ shape(mixed...)).
  (function(shape(...T1, 'x' => int), shape(...T2, 'y' => int)): shape(...T1, ...T2))
  <: (function(shape('a' => bool, 'x' => int), shape('b' => string, 'y' => int)): #1)

[INST-FORALL-L]

(function(shape(...#[T1], 'x' => int), shape(...#[T2], 'y' => int)): shape(...#[T1], ...#[T2]))
  <: (function(shape('a' => bool, 'x' => int), shape('b' => string, 'y' => int)): #1)

[FUN]

shape('a' => bool, 'x' => int) <: shape(...#[T1], 'x' => int)        [1]
shape('b' => string, 'y' => int) <: shape(...#[T2], 'y' => int)      [2]
shape(...#[T1], ...#[T2]) <: #1                                      [3]
```

Propositions `[1]` and `[2]` each have a single type variable in the supertype — fully determined. They produce:

* `#[T1]` gets lower bound `shape('a' => bool)` from `[1]`
* `#[T2]` gets lower bound `shape('b' => string)` from `[2]`

Proposition `[3]` puts both type variables in a multi-splat: `shape(...#[T1], ...#[T2]) <: #1`. The supertype `#1` is a type variable, so the check is handled by the type variable rule (not the shape rule): `#1` gets the splat as its lower bound. The multi-splat is never directly checked as a shape-vs-shape proposition.

Both type variables are fully constrained from their sole-splat parameter occurrences. The multi-splat in the return type is underdetermined on its own, but it doesn't need to be; the solutions come from elsewhere.

##### **Multi-splat as shape-vs-shape**

When the same function is used as a first-class function parameter with a concrete return type, the multi-splat DOES appear as a shape-vs-shape check:

```c#
function merge<T1 as shape(mixed...), T2 as shape(mixed...)>(
  shape(...T1) $a,
  shape(...T2) $b,
): shape(...T1, 'x' => int, ...T2) { ... }

function apply(
  (function(shape('a' => bool), shape('z' => bool)): shape('a' => bool, 'x' => int, 'z' => bool)) $f,
): void { ... }

apply(merge<>);
```

```c#
∀ (T1 ≤ shape(mixed...)) (T2 ≤ shape(mixed...)).
  (function(shape(...T1), shape(...T2)): shape(...T1, 'x' => int, ...T2))
  <: (function(shape('a' => bool), shape('z' => bool)): shape('a' => bool, 'x' => int, 'z' => bool))

[INST-FORALL-L]

(function(shape(...#[T1]), shape(...#[T2])): shape(...#[T1], 'x' => int, ...#[T2]))
  <: (function(shape('a' => bool), shape('z' => bool)): shape('a' => bool, 'x' => int, 'z' => bool))

[FUN]

shape('a' => bool) <: shape(...#[T1])                                                              [1]
shape('z' => bool) <: shape(...#[T2])                                                              [2]
shape(...#[T1], ...shape('x' => int), ...#[T2]) <: shape('a' => bool, 'x' => int, 'z' => bool)     [3]
```

Propositions `[1]` and `[2]` simplify to `shape('a' => bool) <: #[T1]` and `shape('z' => bool) <: #[T2]` (sole splats lift). These give:

* `#[T1] >= shape('a' => bool)`
* `#[T2] >= shape('z' => bool)`

Proposition `[3]` has a shape splat on both sides, so normalization runs. The splat has two type variables (`#[T1]` and `#[T2]`). Normalization replaces `#[T1]` (leftmost) with its current lower bound. Since propositions `[1]`and`[2]` were processed first (function decomposition emits parameters before return), `#[T1]`'s lower bound is `shape('a' => bool)`:

```c#
shape(...shape('a' => bool), ...shape('x' => int), ...#[T2])
```

The two simple shapes merge: `merge(shape('a' => bool), shape('x' => int)) = shape('a' => bool, 'x' => int)`. After normalization, the concrete part is `shape('a' => bool, 'x' => int)` with `#[T2]` rightmost. Now the trick handles the single-type-variable case:

```c#
upper_bound for #[T2]: shape('z' => bool, ?'a' => #[f1], ?'x' => #[f2])
effective_sub = merge(shape('a' => bool, 'x' => int), shape('z' => bool, ?'a' => #[f1], ?'x' => #[f2]))
             = shape('a' => (bool | #[f1]), 'x' => (int | #[f2]), 'z' => bool)

check: shape('a' => (bool|#f1), 'x' => (int|#f2), 'z' => bool) <: shape('a' => bool, 'x' => int, 'z' => bool)
  'a': (bool | #f1) <: bool — #f1 <: bool
  'x': (int | #f2) <: int — #f2 <: int
  'z': bool <: bool
```

Both fresh variables solve to type `nothing`. `#[T2]` gets an upper bound `shape('z' => bool, absent 'a', absent 'x')`, which is consistent with its lower bound `shape('z' => bool)` from proposition `[2]`.

The key: `#[T1]`'s lower bound was already populated when proposition `[3]` was normalized, so the substitution used the actual solution rather than `nothing`.

##### **Incompleteness: when the lower bound isn't yet available**

What if the type variable's lower bound hasn't been populated when the multi-splat is normalized? This can't happen with the current DFS strategy,, but it's worth understanding the limitation.

If `#[T1]`'s lower bound was still `nothing` when proposition \[3\] ran, normalization would substitute `nothing`, which vanishes. All fields would be attributed to `#[T2]`. The fresh-variable technique would give `#[T2]` an upper bound containing ALL fields (including `'a'` which belongs to `#[T1]`). Later, when `#[T2]`'s lower bound arrives from its sole-splat (`shape('z' => bool)`), the bound check `shape('z' => bool) <: shape('a' => bool, 'z' => bool, ...)` would fail;  the lower bound doesn't have `'a'`.

This would be a **false rejection**: the proposition is valid (there exist solutions for both type variables), but the multi-splat reduction attributed fields to the wrong type variable.

With the current implementation, this doesn't happen because:

1. Shape type variables are covariantly observed from their sole-splat parameter checks (right side of `<:`), so they solve to their lower bound
2. The lower bound is populated before the multi-splat is processed
3. The substitution uses the actual lower bound, giving the correct result

A future worklist-based solver that reorders propositions would need to handle this, either by deferring multi-splat normalization until all type variables are constrained, or by re-checking when new bounds arrive.

##### **The well-formedness constraint**

For a polymorphic shape function to be well-formed, every type parameter that appears as a splat element must also appear as the **sole type parameter** in at least one splat across the function's parameters and return type.

To check this, we collect two sets across all parameters and the return type:

* All type parameters that appear inside any splat
* Type parameters that appear as the only type parameter in some splat (other elements like concrete fields are fine; `shape(...T, 'x' => int)` counts as a sole occurrence for `T`)

If the first set minus the second is non-empty, the function is ill-formed — some type parameter has no determined occurrence.

###### *Ill-formed example*

```c#
function process<T1 as shape(mixed...), T2 as shape(mixed...)>(
  shape('flag' => bool, ...T1, ...T2) $data
): void { ... }
```

`T1` and `T2` both appear in `shape('flag' => bool, ...T1, ...T2)`. This splat has two type parameters, so neither is sole. Neither appears anywhere else. The check fails: `{T1, T2} - {} = {T1, T2}`.

###### *Well-formed example*

```c#
function merge<T1 as shape(mixed...), T2 as shape(mixed...)>(
  shape(...T1, 'x' => int) $a,
  shape(...T2, 'y' => int) $b,
): shape(...T1, ...T2) { ... }
```

`T1` appears in `shape(...T1, 'x' => int)` — one type parameter in the splat, so `T1` is sole. `T1` also appears in `shape(...T1, ...T2)` \- two type parameters, not sole. `T2` appears in `shape(...T2, 'y' => int)` \- sole. Also in `shape(...T1, ...T2)` \- not sole.

Both have a sole occurrence: `{T1, T2} - {T1, T2} = {}`. The check passes.

The sole occurrence guarantees that for each type parameter, there exists at least one proposition where it's the only type variable; a fully determined decomposition that constrains it via the fresh-variable technique. The multi-splat occurrence may be underdetermined, but the type variable's solution comes from its sole-splat occurrence.

---

## 5\. Runtime

This proposal requires changes beyond the typechecker. The HHVM runtime and the bytecode emitter must be extended to support shape splats at the value level and to represent splat types in type structures. This work is outside the scope of the mousetrap prototype and will be co-designed with the HHVM team during Phase 3 of the rollout (§2.6). The preliminary positions below are starting points for that design; each may change based on HHVM team input.

### 5.1 Value-Level Shape Splat Emission

The emitter must translate `shape(...$a, ...$b, 'x' => 1)` into bytecode.

We propose introducing a new `MergeDict` bytecode that will initially have a trivial implementation of calling dict merge on the top two stack values (with the stack top being the argument being merged in) and push the resulting dict onto the stack as a primitive to support `...$a`. This will allow future HHVM optimizations while being simple to implement.

### 5.2 Type Structure Representation & Semantics

HHVM's reified generics and `TypeStructure<T>` must be able to represent shape splat types.

Closed shapes are currently represented with a type structure of `dict[“kind” => SHAPE, “fields” => dict[“key” => fieldTS]`. Open shapes have an additional `”allows_unknown_fields” => true` entry in the type structure.

We propose extending the type structure representation to have a “splats” field that contains a vector of either strings or type structures matching the insertion order at declaration time. For instance, given a `shape(“a” => int, …T1, “b” => string, …T2)`, the splats field would be a `vec[“a”, TS(T1), “b”, TS(T2)]`.

For resolved type structures, it will be an error to have a “splats” field. Resolving type structures will follow the type system semantics laid out above \- later elements will win with the same nuances about optional fields in splats. If we prohibit reified shape splat type parameters, this resolution will always result in a concrete shape, potentially with allows\_unknown\_fields if any of the splats are open.

Since `is` and `as` operators operate on type structures, these changes will be sufficient for the operators to work correctly.

**Open questions:**

- Is there a compelling use case for reified shape splat type parameters, or can we prohibit them initially?

### 5.3 TypeAssert and Runtime Enforcement

When a shape splat type appears in an enforced position (parameter type, return type), HHVM must check it at runtime against parsed type constraints (which are distinct from type structures). HHVM must be modified to understand type constraints for shape splats, matching the semantics above, and the rest of the runtime machinery will work uniformly.

**Open questions:**

- How does `supportdyn` interact with shape splat enforcement?

### 5.4 Shapes:: Built-In Functions

Functions like `Shapes::idx`, `Shapes::at`, `Shapes::toDict`, and `Shapes::removeKey` have runtime implementations.

## 6\. Other Considerations

### 6.1 Type Constants

Shape splats interact with Hack's abstract and concrete type constants. The full design will be developed during Phase 2 (§2.6); preliminary positions are given here.

#### 6.1.1 Concrete Type Constants

A class can define a type constant that is a shape with splats:

```c#
class Foo {
  const type TFields = shape('x' => int);
  const type TExtended = shape(...this::TFields, 'y' => string);
}
```

**Preliminary position:** `this::TFields` resolves statically during localization, just as type alias expansion does today. In a subclass that overrides `TFields`, the override's value is substituted. This is consistent with how concrete type constants already work; they are resolved at the point of use, and overrides in subclasses produce different resolved types.

#### 6.1.2 Abstract Type Constants

An abstract type constant bounded by a shape type may appear as a splat element:

```c#
abstract class Base {
  abstract const type TExtra as shape(mixed...);
  const type TFull = shape('id' => int, ...this::TExtra);
}
```

**Preliminary position:** Abstract type constants can be used as splat elements, treated analogously to type parameters. `this::TExtra` behaves like a rigid type variable bounded by `shape(mixed...)` and follows the same subtyping rules (§4.4.3). The sole-splat condition (§4.5.6) applies: `this::TExtra` must appear as the sole splat in at least one position where it can be independently constrained.

### 6.2 HackAST Integration

HackAST is the WWW program transformation framework that provides a typed AST for automated code modifications (codemods). Shape splat types introduce new AST node kinds that HackAST must represent and manipulate.

#### 6.2.1 New AST Nodes

HackAST's type representation must be extended with:

- A splat element node for shape type specifiers (the `...T` in `shape(...T, 'x' => int)`)
- A splat element node for shape expressions (the `...$expr` in `shape(...$expr, 'x' => 1)`)
- An absent field node (`absent 'x'`)


**Preliminary position:** These are straightforward additions to HackAST's node types. The splat element nodes are analogous to existing spread nodes in other expression contexts.

#### 6.2.2 Codemod Support

Codemods that manipulate shape types (e.g., adding/removing fields, refactoring type aliases) must be updated to handle splat elements.

**Preliminary position:** Codemods that add a field to a shape type alias do NOT need to update shapes that splat it; the splat semantics handle this automatically (the splatting shape inherits the new field). This is one of the key benefits of shape splats for maintainability (§1.1). Codemods that inline type aliases must apply splat normalization as part of the inlining.

#### 6.2.3 Pretty-Printing and Round-Tripping

HackAST must be able to pretty-print shape splat types back to valid Hack source and round-trip them through parse → transform → emit without losing information.

**Preliminary position:** The surface syntax (§3) is designed to be unambiguous, so round-tripping should be straightforward. The printer emits `...T` for splat elements and `absent 'f'` for absent fields.

### 6.3 Error Messages

When a splat subtyping check fails, the error message should present types in a form the user can relate to their source code.

**Preliminary position:** Error messages display the **surface syntax** form, not the internal normalized representation. For example, if `shape(...User, 'x' => int)` fails to subtype `shape('x' => string)`, the error should reference `shape(...User, 'x' => int)`, not the fully expanded field list. The `s_origin` field (preserved through normalization) provides the information needed to reconstruct the surface form. For fields inherited through splats, the error should indicate which splat element contributed the conflicting field.

### 6.4 IDE Support

Shape splats affect IDE features including autocomplete, hover types, and go-to-definition.

**Preliminary position:** Autocomplete for a value of type `shape(...User, 'x' => int)` should suggest all fields from `User` plus `'x'`. This requires resolving the splat to its constituent fields, which localization already does. Hover types should display the surface syntax form (matching error messages, §6.3). Go-to- definition for a field inherited through a splat should navigate to the field's definition in the splatted type alias, not to the splat site.

### 6.5 Migration

Expression-level shape splats (Phase 3\) provide a type-safe replacement for `Shape\fb\merge`.

**Preliminary position:** After Phase 3 is stable, `Shape\fb\merge` should be soft-deprecated in favor of expression-level splats. A codemod can mechanically transform `Shape\fb\merge($a, $b, $c)` to `shape(...$a, ...$b, ...$c)`. The codemod is semantics-preserving (both have rightmost-wins behavior) and the splat form provides strictly more precise types. The deprecation and codemod should be deferred until Phase 3 is validated in production.

## 7\. Prior Art & Design Comparison

Shape splats draw on a rich body of work on row polymorphism, record calculi, and typed merge operators. This section surveys the key prior art and clarifies what we borrow, what we depart from, and why. Each subsection ends with a "borrowed / not borrowed" summary.

### 7.1 Classical Row Polymorphism

**Wand (1987), "Complete Type Inference for Simple Objects"** The foundational paper on row variables. A record type `{x: int | ρ}` has a single extension point `ρ` representing "the rest of the fields". Inference works by unification over row variables, giving complete type inference for record operations (access, extension, restriction).

**Rémy (1989; 1994), "Type Checking Records and Variants in a Natural Extension of ML"** Extends Wand's row variables with per-field **presence/absence flags**. A row explicitly tracks which fields are present (`pre`) and which are absent (`abs`): `{x: pre int; y: abs; ρ}`. This enables **`lacks` constraints** ("ρ lacks x" means x cannot appear in ρ), which are checked at instantiation. Rémy's system is the basis for OCaml's object types and PureScript's row system.

**Borrowed:**

- The concept of an extension variable for "the rest of the fields." Our splat type parameters (`...T`) serve the same role as Wand's row variables: they represent unknown fields that are preserved through operations on known fields.
- Presence/absence tracking. Our `Req`/`Opt`/`Absent` lattice generalizes Rémy's binary `pre`/`abs` flag by carrying a type: `Opt τ` means "if present, has type τ," whereas Rémy's `abs` is simply "not present."

**Not borrowed:**

- **Single extension point.** Classical row types have exactly one row variable per record. We allow multiple splats (`shape(...T, 'x' => int, ...U)`), enabling multi-source composition. This is a deliberate departure: a single row variable cannot express "T's fields, then some concrete fields, then U's fields" with positional override semantics.
- **Mandatory disjointness.** In Rémy's system, a row variable cannot mention a field already present in the record; extending `{x: int | ρ}` requires `ρ lacks x`. We use rightmost-wins by default, with opt-in disjointness planned as a future extension. This matches Hack's value-level merge semantics and enables the defaults pattern (§4.1.3).

### 7.2 Record Subtyping and Concatenation

**Cardelli (1992), "Extensible Records in a Pure Calculus of Subtyping"** Defines record types with width and depth subtyping in a calculus without row variables. Records support a merge operator `|` that is **symmetric** (order-independent) and requires **disjointness** (overlapping fields are a type error). Cardelli also discussed **concatenation-with-override** in earlier work ("Operations on Records," DEC SRC Report 48, 1990), where the right operand's fields take precedence over the left's. This is the closest classical precedent to our rightmost-wins semantics, though Cardelli did not develop inference or polymorphism for the asymmetric variant.

**Harper and Pierce (1991), "A Record Calculus Based on Symmetric Concatenation"** A record calculus where the concatenation operator is symmetric and requires disjointness, enforced through constraints on row variables. Overlapping fields make the constraint system unsolvable, rejecting the program.

**Borrowed:**

- The idea of a merge/concatenation operator on record types as a first-class type-level operation.

**Not borrowed:**

- **Symmetric semantics.** Both Cardelli and Harper-Pierce treat concatenation as commutative (or require disjointness, making commutativity trivial). We are deliberately asymmetric: `shape(...A, ...B) ≠ shape(...B, ...A)` when A and B share fields. This matches the runtime semantics of `{...a, ...b}` in JavaScript/Hack.
- **Disjointness-required.** Both systems reject overlapping fields as errors. We allow overlap with rightmost-wins resolution, reserving disjointness as an opt-in safety mechanism for future work.

### 7.3 Constraint-Based Rows (Pottier 2003\)

**Pottier (2003), "A Constraint-Based Presentation and Generalization of Rows"** Reformulates row polymorphism as a **constraint satisfaction problem**. Rather than treating rows as type constructors with special unification rules, Pottier models them as constraints on record structure. This separates constraint generation (during type checking) from constraint solving (during inference), making the solving strategy pluggable.

**Borrowed:**

- Our inference approach (§4.5) is constraint-based in the same spirit: type variables accumulate upper and lower bounds as subtyping constraints are solved, and the inferred type is the tightest type consistent with all bounds. The separation of constraint generation (subtyping rules emit bounds on type variables) from constraint solving (the handler resolves bounds) in the mousetrap prototype directly follows Pottier's architecture.

**Not borrowed:**

- The single-row-variable model. Pottier's constraints describe a single row; our constraints describe the interaction of multiple splat elements within a single shape.

### 7.4 Scoped Labels (Leijen 2005\)

**Leijen (2005), "Extensible Records with Scoped Labels"** Extends row polymorphism to allow **duplicate field labels** resolved by scope. A record can have multiple bindings for the same label at different "depths"; field access returns the outermost (most recently added) binding. Extension adds a new binding that shadows earlier ones; restriction removes the outermost binding, revealing the one beneath. This system is used in the Koka language for effect rows.

This is the closest theoretical precedent to our rightmost-wins semantics. Each splat in `shape(...A, ...B)` can be viewed as a "layer": B's bindings shadow A's for shared fields, and field access resolves to the rightmost (outermost) layer. The key conceptual parallel is that a field can have **multiple definitions resolved by position**, not just one definition with a disjointness requirement.

**Borrowed:**

- The core insight that duplicate definitions of a field are not necessarily an error; they can be resolved by position. Our rightmost-wins merge is a simplified version of Leijen's scoped resolution.

**Not borrowed:**

- **The full scoping mechanism.** Leijen's system tracks scope depth and supports restriction (removing the outermost binding to reveal the one beneath). We have no analog of restriction; our merge is one-directional and irreversible after normalization.
- **Single row variable.** Leijen's system still uses one row variable per record, extended with scope tracking. We allow multiple independent splat elements.

### 7.5 Abstract Row Theories (Morris and McKinna 2019\)

**Morris and McKinna (2019), "An Abstract Type System for Row Theories" (Rose).** Provides an **abstract framework** that parameterizes over the row algebra. Different concrete row systems (Rémy's flags, Leijen's scoped labels, effect rows) are shown to be instances of the same abstract machinery. The framework uses **qualified types** with predicates such as `lacks` (a field is absent from a row) and `combination` (how two rows merge).

**Borrowed:**

- Our `absent` field is analogous to Rose's `lacks` predicate: both declare that a specific field is not present.
- The recognition (from Rose's abstract treatment) that different combination operators (symmetric, asymmetric, scoped) are valid instantiations of the same abstract structure. Our rightmost-wins merge is another such instantiation.

**Not borrowed:**

- We do not formalize within Rose's framework, though our system could plausibly be cast as an instance with rightmost-wins as the row combination operator and `disjoint` as a qualified-type predicate.

### 7.6 Typed Merge and Intersection Types

**Bao et al. (2023), "Taming the Merge Operator"** Studies typing of a general-purpose merge operator in the context of intersection types and the λi calculus. Their merge `e₁ ,, e₂` produces a value of intersection type `A & B`; field access on an intersection type must resolve ambiguity (e.g., via specificity or explicit annotation). The focus is on general term-level merge, not record-specific composition.

**Not borrowed:**

- **Intersection-type semantics.** Their merge produces an intersection type where both components coexist; ours produces a single shape type where the rightmost component wins. These are different points in the design space: intersection preserves all information (at the cost of ambiguity resolution), while rightmost-wins discards the overridden information (at the cost of irreversibility).

### 7.7 Practical Systems

Several production languages implement aspects of row polymorphism or record composition, providing practical context for our design.

**TypeScript.** Object spread syntax (`{...a, ...b}`) has rightmost-wins semantics at runtime, matching our proposal exactly. However, TypeScript's type system handles spreads 'pragmatically' rather than through a principled row system.

*Overlapping fields.* TypeScript computes spread types by taking later properties over earlier ones for known types, but loses precision when types are generic:

```ts
// TypeScript
function mergeGeneric<T extends object, U extends object>(a: T, b: U): T & U {
  return { ...a, ...b } as T & U;
}

// const resultGeneric: never
const resultGeneric = mergeGeneric({ x: 1, y: true }, { y: 'nope', z: 42 });


function mergeConcrete(
  a: {x: number; y: boolean;},
  b: {y: string; z: number}): {x: number; y: string; z:number} {
   return { ...a, ...b };
}

// const resultConcrete: {x: number; y: string; z: number;}
const resultConcrete = mergeConcrete({ x: 1, y: true }, { y: 'nope', z: 42 });
```

```c#
// Shape splat
$result = shape(...$a, ...$b);
// shape('x' => int, 'y' => string, 'z' => int)
// 'y' is string (rightmost wins), matching runtime
```

TypeScript models generic spread as intersection (`T & U`), which treats overlapping fields as `boolean & string` \= `never`. The actual runtime value is `string` (rightmost wins). Our merge algebra (§4.1.3) computes the precise type directly.

*Defaults pattern.* A common idiom is merging default values with caller-provided overrides:

```ts
// TypeScript
type Defaults = { pageSize: number; timeout: number };
type Options = { timeout?: number; retries?: number };

function configure(opts: Options): Defaults & Options {
  return { pageSize: 10, timeout: 30, ...opts };
}
// Return type: Defaults & Options
// 'timeout' is number & (number | undefined) = number
// Works, but only because both sides happen to have 'number'
// If the types differed, intersection would produce 'never'
```

```c#
// Shape splat
function configure(
  shape(?'timeout' => int, ?'retries' => int) $opts,
): shape('pageSize' => int, 'timeout' => int, ?'retries' => int) {
  return shape('pageSize' => 10, 'timeout' => 30, ...$opts);
}
// Merge: (Req int, Opt int) → Req (int | int) = Req int
// Precise: 'timeout' is required int (default guarantees presence)
```

Our merge algebra handles the defaults pattern precisely: the default's `Req int` merged with the override's `Opt int` produces `Req int` (required, because the default guarantees presence).

**Flow.** Facebook's Flow type checker supports `...` spread in exact object types (`{| |}`) at both the value level and the type level. For concrete types, Flow computes spread types precisely with rightmost-wins semantics:

```javascript
// Flow; concrete spread is precise
type Defaults = {| pageSize: number, timeout: number |};
type Options = {| timeout?: number, retries?: number |};

const defaults: Defaults = { pageSize: 10, timeout: 30 };
const options: Options = { timeout: 60 };

const config = { ...defaults, ...options };
// Flow infers: {| pageSize: number, timeout: number, retries?: number |}
// Precise: 'timeout' is number (rightmost-wins), 'pageSize' preserved
```

Flow also supports spread at the type level for composing object type aliases:

```javascript
// Flow; type-level spread
type User = {| id: string, name: string |};
type TimestampedUser = {| ...User, createdAt: number |};
// Equivalent to: {| id: string, name: string, createdAt: number |}
```

However, Flow's type-level spread has limitations with polymorphism. A generic function that adds a field cannot use spread in its return type annotation to express "T plus an extra field" with full inference:

```javascript
// Flow; polymorphic spread is limited
function addTimestamp<T: {...}>(obj: T): {| ...T, createdAt: number |} {
  return { ...obj, createdAt: Date.now() };
  // Error: Cannot return object literal because inexact object literal [1] is incompatible with exact object type [2]. [incompatible-exact]
}
```

```c#
// Shape splat; polymorphic spread with inference
function add_timestamp<T as shape(mixed...)>(
  shape(...T) $s,
): shape(...T, 'created_at' => int) {
  return shape(...$s, 'created_at' => time());
}
// T is inferred from the argument via sole-splat inference (§4.5)
// Return type is computed precisely with no annotation needed on T
```

Flow's concrete spread typing is the closest existing system to our proposal; it correctly handles rightmost-wins for known types and supports type-level composition. The gap is in polymorphic inference: Flow's type-level spread is a built-in operation with special-case handling, not a general mechanism with constraint-based inference. Our shape splats integrate spread into the type system with first-class inference (§4.5) and the sole-splat well-formedness condition (§4.5.6).

**PureScript.** Implements Rémy-style row polymorphism with explicit `lacks` constraints. Records have a single row variable; extension requires the row to lack the field being added. This is the closest practical implementation of classical row theory.

```rust
-- PureScript: adding a field requires a 'lacks' constraint
addTimestamp :: forall r. Lacks "createdAt" r =>
  Record r -> Record (createdAt :: Int | r)
addTimestamp rec = Record.insert (Proxy :: Proxy "createdAt") (now unit) rec
```

```c#
// Shape splat: absent field on the bound achieves the same effect
function add_timestamp<T as shape(absent 'created_at', mixed...)>(
  shape(...T) $s,
): shape(...T, 'created_at' => int) {
  return shape(...$s, 'created_at' => time());
}
```

PureScript's `Lacks "createdAt" r` is analogous to our `absent 'created_at'` on the type parameter bound: both ensure `T`/`r` does not already have the field being added. The key differences are that PureScript uses a single row variable (no multi-source composition) and requires disjointness (no rightmost-wins override). In our system, the `absent` constraint is opt-in; without it, the function would still type-check, and `T`'s `'created_at'` (if any) would be silently overridden.

**Elm.** Simplified row polymorphism with extensible record syntax (`{ a | x : Int }` meaning "a record with at least field x"). Record update (`{ r | x = 5 }`) is a restricted form of field override on a single record. Inference is simpler than PureScript's but less expressive; there are no `lacks` constraints or explicit absence tracking.

**OCaml.** Row polymorphism appears in two forms: object types (`< x: int; .. >` where `..` is an implicit row variable) and polymorphic variants (``[`A of int | `B of string | .. ]``). Both use Rémy-style inference internally. OCaml's row polymorphism is mature and battle-tested but limited to single row variables and does not support record concatenation.

### 7.8 Summary

| System | Extension model | Merge semantics | Overlap handling | Multiple extension points | Disjointness mechanism |
| :---- | :---- | :---- | :---- | :---- | :---- |
| Wand 1987 | Single row var | N/A (extend only) | Implicit lacks | No | Implicit |
| Rémy 1989 | Single row var \+ flags | N/A (extend only) | Explicit lacks | No | Lacks constraint |
| Cardelli 1992 | Subtyping | Symmetric merge | Error (disjoint required) | Yes | Required |
| Harper-Pierce 1991 | Single row var | Symmetric merge | Error (constraints) | Yes (if disjoint) | Constraints |
| Pottier 2003 | Single row var | Constraints | Via constraints | No | Constraints |
| Leijen 2005 | Scoped row var | Scoped override | Shadowing by scope | By scope layers | Scope separation |
| Rose 2019 | Abstract /parameterized | Parameterized | System-defined | Flexible | Qualified types |
| TypeScript | Structural types | Rightmost-wins (ad-hoc) | Imprecise typing | Yes | None |
| PureScript | Single row var (Rémy) | N/A (extend only) | Lacks constraint | No | Lacks constraint |
| **Shape splat** | **Multiple splats** | **Rightmost-wins** | **Override**  | **Yes** | **`absent` fields, `disjoint_fields` as future work**  |

Our design occupies a distinctive point in this space: rightmost-wins merge with multiple extension points, combining the expressiveness of concatenation calculi (Cardelli, Harper-Pierce) with the polymorphism of row-variable systems (Wand, Rémy), while using positional override semantics closest to Leijen's scoped labels. The future, opt-in `disjoint_fields` constraint bridges to the classical disjointness-required world when safety is needed.


# Shape splat inference: All Cases

## Overview

After normalization, both sides of a shape subtype check are in `normalized_shape` form:

* Concrete:  a simple shape, no type variables
* Type variable leftmost:  concrete part is rightmost (definitive)
* Type variable rightmost:  type variable is rightmost (dominant)

This gives a 3x3 matrix. For each case, overlapping fields (present in both the concrete part and the other side) may be `Req` or `Opt`. The tables below show every combination with a concrete example proposition, along with all resulting sub-propositions.

Each proposition uses `'x'` as the overlapping field, with additional fields (`'y'`, `'z'`) to show how non-overlapping fields are routed to type variable bounds.

---

## No fresh variable (concrete splat is definitive)

When the type variable is leftmost, the concrete part is rightmost and definitive for overlapping fields. The check is a direct constraint;  no fresh variables needed.

### Type variable in supertype, leftmost

The supertype is `shape(...#v, ...shape(concrete_fields))`. Concrete is rightmost (definitive). Subtype-only fields appear in `#v`'s lower bound.

| Concrete supertype | Subtype | Proposition | Overlapping 'x' | Test |
| :---- | :---- | :---- | :---- | :---- |
| Req | Req | `shape('x' => int, 'y' => bool) <: shape(...#v, ...shape('x' => int))` | `int <: int` | `super_left_rr` |
| Req | Opt | `shape(?'x' => int, 'y' => bool) <: shape(...#v, ...shape('x' => int))` | **FAIL** (Opt \</: Req) | `super_left_ro` |
| Opt | Req | `shape('x' => int, 'y' => bool) <: shape(...#v, ...shape(?'x' => int))` | `int <: int` | `super_left_or` |
| Opt | Opt | `shape(?'x' => int, 'y' => bool) <: shape(...#v, ...shape(?'x' => int))` | `int <: int` | `super_left_oo` |

**Req/Req**: `shape('x' => int, 'y' => bool) <: shape(...#v, ...shape('x' => int))`

* unknown: `nothing <: nothing`
* `'x'`: both, concrete definitive \-- `Req int <: Req int` \-- `int <: int`
* `'y'`: only in subtype \-- `#v >= shape('y' => bool)`

**Req/Opt** (FAIL): `shape(?'x' => int, 'y' => bool) <: shape(...#v, ...shape('x' => int))`

* `'x'`: `Opt int <: Req int` \-- **FAIL** (Opt \</: Req). Concrete supertype requires `'x'` but subtype has it optionally.

**Opt/Req**: `shape('x' => int, 'y' => bool) <: shape(...#v, ...shape(?'x' => int))`

* unknown: `nothing <: nothing`
* `'x'`: both, concrete definitive \-- `Req int <: Opt int` \-- `int <: int` (Req \<: Opt fine)
* `'y'`: only in subtype \-- `#v >= shape('y' => bool)`

**Opt/Opt**: `shape(?'x' => int, 'y' => bool) <: shape(...#v, ...shape(?'x' => int))`

* unknown: `nothing <: nothing`
* `'x'`: both, concrete definitive \-- `Opt int <: Opt int` \-- `int <: int`
* `'y'`: only in subtype \-- `#v >= shape('y' => bool)`

### Type variable in subtype, leftmost

The subtype is `shape(...#v, ...shape(concrete_fields))`. Concrete is rightmost (definitive). Supertype-only fields appear in `#v`'s upper bound.

| Concrete subtype | Supertype | Proposition | Overlapping 'x' | Test |
| :---- | :---- | :---- | :---- | :---- |
| Req | Req | `shape(...#v, ...shape('x' => int)) <: shape('x' => int, 'z' => bool)` | `int <: int` | `sub_left_rr` |
| Req | Opt | `shape(...#v, ...shape('x' => int)) <: shape(?'x' => int, 'z' => bool)` | `int <: int` | `sub_left_ro` |
| Opt | Req | `shape(...#v, ...shape(?'x' => int)) <: shape('x' => int, 'z' => bool)` | **FAIL** (Opt \</: Req) | `sub_left_or` |
| Opt | Opt | `shape(...#v, ...shape(?'x' => int)) <: shape(?'x' => int, 'z' => bool)` | `int <: int` | `sub_left_oo` |

**Req/Req**: `shape(...#v, ...shape('x' => int)) <: shape('x' => int, 'z' => bool)`

* unknown: `nothing <: nothing`
* `'x'`: both, concrete definitive \-- `Req int <: Req int` \-- `int <: int`
* `'z'`: only in supertype \-- `#v ≤ shape('z' => bool)`

**Req/Opt**: `shape(...#v, ...shape('x' => int)) <: shape(?'x' => int, 'z' => bool)`

* unknown: `nothing <: nothing`
* `'x'`: both, concrete definitive \-- `Req int <: Opt int` \-- `int <: int` (Req \<: Opt fine)
* `'z'`: only in supertype \-- `#v ≤ shape('z' => bool)`

**Opt/Req** (FAIL): `shape(...#v, ...shape(?'x' => int)) <: shape('x' => int, 'z' => bool)`

* `'x'`: `Opt int <: Req int` \-- **FAIL** (Opt \</: Req). Concrete subtype has `'x'` optionally but supertype requires it. Since concrete splat is definitive, the type variable can't fix this.

**Opt/Opt**: `shape(...#v, ...shape(?'x' => int)) <: shape(?'x' => int, 'z' => bool)`

* unknown: `nothing <: nothing`
* `'x'`: both, concrete definitive \-- `Opt int <: Opt int` \-- `int <: int`
* `'z'`: only in supertype \-- `#v ≤ shape('z' => bool)`

### Type variables in both, both leftmost

Both concretes are rightmost (definitive). Direct field-by-field. Subtype-only concrete fields appear in supertype's type variable's lower bound. Supertype-only concrete fields appear in subtype's type variable’s upper bound.

| Concrete subtype | Concrete supertype | Proposition | Overlapping 'x' | Test |
| :---- | :---- | :---- | :---- | :---- |
| Req | Req | `shape(...#v1, ...shape('x' => int, 'y' => bool)) <: shape(...#v2, ...shape('x' => int, 'z' => string))` | `int <: int` | `both_left_rr` |
| Req | Opt | `shape(...#v1, ...shape('x' => int, 'y' => bool)) <: shape(...#v2, ...shape(?'x' => int, 'z' => string))` | `int <: int` | `both_left_ro` |
| Opt | Req | `shape(...#v1, ...shape(?'x' => int, 'y' => bool)) <: shape(...#v2, ...shape('x' => int, 'z' => string))` | **FAIL** (Opt \</: Req) | `both_left_or` |
| Opt | Opt | `shape(...#v1, ...shape(?'x' => int, 'y' => bool)) <: shape(...#v2, ...shape(?'x' => int, 'z' => string))` | `int <: int` | `both_left_oo` |

**Req/Req**: `shape(...#v1, ...shape('x' => int, 'y' => bool)) <: shape(...#v2, ...shape('x' => int, 'z' => string))`

* unknown: `nothing <: nothing`
* `'x'`: both, both definitive \-- `Req int <: Req int` \-- `int <: int`
* `'y'`: only in concrete subtype \-- `#v2 ≥ shape('y' => bool)`
* `'z'`: only in concrete supertype \-- `#v1 ≤ shape('z' => string)`

**Req/Opt**: `shape(...#v1, ...shape('x' => int, 'y' => bool)) <: shape(...#v2, ...shape(?'x' => int, 'z' => string))`

* unknown: `nothing <: nothing`
* `'x'`: both, both definitive \-- `Req int <: Opt int` \-- `int <: int` (Req \<: Opt fine)
* `'y'`: only in concrete subtype \-- `#v2 ≥ shape('y' => bool)`
* `'z'`: only in concrete supertype \-- `#v1 ≤ shape('z' => string)`

**Opt/Req** (FAIL): `shape(...#v1, ...shape(?'x' => int, 'y' => bool)) <: shape(...#v2, ...shape('x' => int, 'z' => string))`

* `'x'`: `Opt int <: Req int` \-- **FAIL** (Opt \</: Req). Both concretes are definitive; the subtype's concrete splat has `'x'` optionally but the supertype's concrete splat requires it.

**Opt/Opt**: `shape(...#v1, ...shape(?'x' => int, 'y' => bool)) <: shape(...#v2, ...shape(?'x' => int, 'z' => string))`

* unknown: `nothing <: nothing`
* `'x'`: both, both definitive \-- `Opt int <: Opt int` \-- `int <: int`
* `'y'`: only in concrete subtype \-- `#v2 ≥ shape('y' => bool)`
* `'z'`: only in concrete supertype \-- `#v1 ≤ shape('z' => string)`

---

## Fresh variable on supertype side (lower bound on supertype's type variable)

When the type variable is rightmost in the supertype, it might override the concrete part's fields. Fresh type variable is introduced with optional requiredness.

### Type variable in supertype, rightmost

The supertype is `shape(...shape(concrete_fields), ...#v)`. Type variable rightmost (dominant). Subtype-only fields appear in `#v`'s lower bound. Concrete-only fields remain in the effective supertype via the merge.

| Concrete supertype | Subtype | Proposition | Overlapping 'x' | Test |
| :---- | :---- | :---- | :---- | :---- |
| Req | Req | `shape('x' => int, 'y' => bool) <: shape(...shape('x' => int), ...#v)` | `int <: (int | #f)` | `super_right_rr` |
| Req | Opt | `shape(?'x' => int, 'y' => bool) <: shape(...shape('x' => int), ...#v)` | **FAIL** (Opt \</: Req) | `super_right_ro` |
| Opt | Req | `shape('x' => int, 'y' => bool) <: shape(...shape(?'x' => int), ...#v)` | `int <: (int | #f)` | `super_right_or` |
| Opt | Opt | `shape(?'x' => int, 'y' => bool) <: shape(...shape(?'x' => int), ...#v)` | `int <: (int | #f)` | `super_right_oo` |

**Req/Req**: `shape('x' => int, 'y' => bool) <: shape(...shape('x' => int), ...#v)`

* `#v ≥ shape('y' => bool, ?'x' => #f)` (lower bound: subtype-only fields \+ fresh var for overlapping)
* effective\_super \= `merge(shape('x' => int), shape('y' => bool, ?'x' => #f))` \= `shape('x' => (int | #f), 'y' => bool)`
* unknown: `nothing <: nothing`
* `'x'`: `Req int <: Req (int | #f)` \-- `int <: (int | #f)`
* `'y'`: `Req bool <: Req bool` \-- `bool <: bool`

**Req/Opt** (FAIL): `shape(?'x' => int, 'y' => bool) <: shape(...shape('x' => int), ...#v)`

* `#v ≥ shape('y' => bool, ?'x' => #f)` (lower bound)
* effective\_super \= `merge(shape('x' => int), shape('y' => bool, ?'x' => #f))` \= `shape('x' => (int | #f), 'y' => bool)`
* `'x'`: `Opt int <: Req (int | #f)` \-- **FAIL** (Opt \</: Req). The effective supertype has Req `'x'` (merge preserves concrete's Req) but the subtype has it optionally.

**Opt/Req**: `shape('x' => int, 'y' => bool) <: shape(...shape(?'x' => int), ...#v)`

* `#v ≥ shape('y' => bool, ?'x' => #f)` (lower bound)
* effective\_super \= `merge(shape(?'x' => int), shape('y' => bool, ?'x' => #f))` \= `shape(?'x' => (int | #f), 'y' => bool)`
* unknown: `nothing <: nothing`
* `'x'`: `Req int <: Opt (int | #f)` \-- `int <: (int | #f)` (Req \<: Opt fine)
* `'y'`: `Req bool <: Req bool` \-- `bool <: bool`

**Opt/Opt**: `shape(?'x' => int, 'y' => bool) <: shape(...shape(?'x' => int), ...#v)`

* `#v ≥ shape('y' => bool, ?'x' => #f)` (lower bound)
* effective\_super \= `merge(shape(?'x' => int), shape('y' => bool, ?'x' => #f))` \= `shape(?'x' => (int | #f), 'y' => bool)`
* unknown: `nothing <: nothing`
* `'x'`: `Opt int <: Opt (int | #f)` \-- `int <: (int | #f)`
* `'y'`: `Req bool <: Req bool` \-- `bool <: bool`

### Type variables in both, subtype leftmost, supertype rightmost

Subtype's concrete splat is definitive. Supertype's type variable is rightmost (dominant). Fresh variable on the supertype side for overlapping fields. Subtype-only concrete fields  appear in supertype's type variable's lower bound. Supertype-only concrete fields  appear in subtype's type variable's upper bound.

| Concrete subtype | Concrete supertype | Proposition | Overlapping 'x' | Test |
| :---- | :---- | :---- | :---- | :---- |
| Req | Req | `shape(...#v1, ...shape('x' => int, 'y' => bool)) <: shape(...shape('x' => int, 'z' => string), ...#v2)` | `int <: (int | #f)` | `both_lr_rr` |
| Opt | Req | `shape(...#v1, ...shape(?'x' => int, 'y' => bool)) <: shape(...shape('x' => int, 'z' => string), ...#v2)` | **FAIL** (Opt \</: Req) | `both_lr_or` |
| Req | Opt | `shape(...#v1, ...shape('x' => int, 'y' => bool)) <: shape(...shape(?'x' => int, 'z' => string), ...#v2)` | `int <: (int | #f)` | `both_lr_ro` |
| Opt | Opt | `shape(...#v1, ...shape(?'x' => int, 'y' => bool)) <: shape(...shape(?'x' => int, 'z' => string), ...#v2)` | `int <: (int | #f)` | `both_lr_oo` |

**Req/Req**: `shape(...#v1, ...shape('x' => int, 'y' => bool)) <: shape(...shape('x' => int, 'z' => string), ...#v2)`

* unknown: `nothing <: nothing`
* `'x'`: both concretes have it; supertype's type variable might override \-- fresh `#f` in `#v2`'s lower bound
  * `#v2 ≥ shape(?'x' => #f)`
  * effective supertype field: `merge(Req int, Opt #f)` \= `Req (int | #f)`
  * `Req int <: Req (int | #f)` \-- `int <: (int | #f)`
* `'y'`: only in concrete subtype \-- `#v2 ≥ shape('y' => bool)`
* `'z'`: only in concrete supertype \-- `#v1 ≤ shape('z' => string)`

**Opt/Req** (FAIL): `shape(...#v1, ...shape(?'x' => int, 'y' => bool)) <: shape(...shape('x' => int, 'z' => string), ...#v2)`

* `'x'`: effective supertype field: `merge(Req int, Opt #f)` \= `Req (int | #f)`. Subtype's concrete splat is definitive with `Opt int`. `Opt int <: Req (int | #f)` \-- **FAIL** (Opt \</: Req). The subtype's concrete splat has `'x'` optionally but the effective supertype requires it.

**Req/Opt**: `shape(...#v1, ...shape('x' => int, 'y' => bool)) <: shape(...shape(?'x' => int, 'z' => string), ...#v2)`

* unknown: `nothing <: nothing`
* `'x'`: effective supertype field: `merge(Opt int, Opt #f)` \= `Opt (int | #f)`. `Req int <: Opt (int | #f)` \-- `int <: (int | #f)` (Req \<: Opt fine)
* `'y'`: only in concrete subtype \-- `#v2 ≥ shape('y' => bool)`
* `'z'`: only in concrete supertype \-- `#v1 ≤ shape('z' => string)`

**Opt/Opt**: `shape(...#v1, ...shape(?'x' => int, 'y' => bool)) <: shape(...shape(?'x' => int, 'z' => string), ...#v2)`

* unknown: `nothing <: nothing`
* `'x'`: effective supertype field: `merge(Opt int, Opt #f)` \= `Opt (int | #f)`. `Opt int <: Opt (int | #f)` \-- `int <: (int | #f)`
* `'y'`: only in concrete subtype \-- `#v2 ≥ shape('y' => bool)`
* `'z'`: only in concrete supertype \-- `#v1 ≤ shape('z' => string)`

---

## Fresh variable on subtype side (upper bound on subtype's type variable) \-- the dual

When the type variable is rightmost in the subtype, it might override the concrete part's fields. Fresh type variable is introduced with required requiredness (`Req #f`). Since `mergeFieldDesc(anything, Req #f) = Req #f`, the concrete part to the left is irrelevant — the fresh variable completely dominates. This is dual to the supertype side, which uses `Opt #f`.

### Type variable in subtype, rightmost

The subtype is `shape(...shape(concrete_fields), ...#v)`. Type variable rightmost (dominant). Supertype-only fields go directly to `#v`'s upper bound. Concrete-only fields remain in the effective subtype via the merge.

| Concrete subtype | Supertype | Fresh | Proposition | Overlapping 'x' | Test |
| :---- | :---- | :---- | :---- | :---- | :---- |
| Req | Req | Req \#f | `shape(...shape('x' => int), ...#v) <: shape('x' => int, 'z' => bool)` | `#f <: int` | `sub_right_rr` |
| Req | Opt | Req \#f | `shape(...shape('x' => int), ...#v) <: shape(?'x' => int, 'z' => bool)` | `#f <: int` | `sub_right_ro` |
| Opt | Req | Req \#f | `shape(...shape(?'x' => int), ...#v) <: shape('x' => int, 'z' => bool)` | `#f <: int` | `sub_right_or` |
| Opt | Opt | Req \#f | `shape(...shape(?'x' => int), ...#v) <: shape(?'x' => int, 'z' => bool)` | `#f <: int` | `sub_right_oo` |

**Req/Req**: `shape(...shape('x' => int), ...#v) <: shape('x' => int, 'z' => bool)`

* `#v ≤ shape('z' => bool, 'x' => #f)` (upper bound: supertype-only fields \+ fresh var for overlapping)
* effective\_sub \= `merge(shape('x' => int), shape('z' => bool, 'x' => #f))` \= `shape('x' => #f, 'z' => bool)`
* unknown: `nothing <: nothing`
* `'x'`: `Req #f <: Req int` \-- `#f <: int`
* `'z'`: `Req bool <: Req bool` \-- `bool <: bool`

**Req/Opt**: `shape(...shape('x' => int), ...#v) <: shape(?'x' => int, 'z' => bool)`

* `#v ≤ shape('z' => bool, 'x' => #f)` (upper bound)
* effective\_sub \= `merge(shape('x' => int), shape('z' => bool, 'x' => #f))` \= `shape('x' => #f, 'z' => bool)`
* unknown: `nothing <: nothing`
* `'x'`: `Req #f <: Opt int` \-- `#f <: int` (Req \<: Opt fine)
* `'z'`: `Req bool <: Req bool` \-- `bool <: bool`

**Opt/Req**: `shape(...shape(?'x' => int), ...#v) <: shape('x' => int, 'z' => bool)`

* `#v ≤ shape('z' => bool, 'x' => #f)` (upper bound)
* effective\_sub \= `merge(shape(?'x' => int), shape('z' => bool, 'x' => #f))` \= `shape('x' => #f, 'z' => bool)` (Req from right overrides Opt from left)
* unknown: `nothing <: nothing`
* `'x'`: `Req #f <: Req int` \-- `#f <: int`
* `'z'`: `Req bool <: Req bool` \-- `bool <: bool`

**Opt/Opt**: `shape(...shape(?'x' => int), ...#v) <: shape(?'x' => int, 'z' => bool)`

* `#v ≤ shape('z' => bool, 'x' => #f)` (upper bound)
* effective\_sub \= `merge(shape(?'x' => int), shape('z' => bool, 'x' => #f))` \= `shape('x' => #f, 'z' => bool)` (Req from right overrides Opt from left)
* unknown: `nothing <: nothing`
* `'x'`: `Req #f <: Opt int` \-- `#f <: int` (Req \<: Opt fine)
* `'z'`: `Req bool <: Req bool` \-- `bool <: bool`

### Type variables in both, subtype rightmost, supertype leftmost

Subtype's type variable is rightmost (dominant). Supertype's concrete splat is definitive. Fresh variable on the subtype side. Effective subtype fields checked directly against the definitive supertype concrete. Effective-subtype-only fields  appear in supertype's type variable's lower bound.

| Concrete subtype | Concrete supertype | Fresh | Proposition | Overlapping 'x' | Test |
| :---- | :---- | :---- | :---- | :---- | :---- |
| Req | Req | Req \#f | `shape(...shape('x' => int, 'y' => bool), ...#v1) <: shape(...#v2, ...shape('x' => int, 'z' => string))` | `#f <: int` | `both_rl_rr` |
| Req | Opt | Req \#f | `shape(...shape('x' => int, 'y' => bool), ...#v1) <: shape(...#v2, ...shape(?'x' => int, 'z' => string))` | `#f <: int` | `both_rl_ro` |
| Opt | Req | Req \#f | `shape(...shape(?'x' => int, 'y' => bool), ...#v1) <: shape(...#v2, ...shape('x' => int, 'z' => string))` | `#f <: int` | `both_rl_or` |
| Opt | Opt | Req \#f | `shape(...shape(?'x' => int, 'y' => bool), ...#v1) <: shape(...#v2, ...shape(?'x' => int, 'z' => string))` | `#f <: int` | `both_rl_oo` |

**Req/Req**: `shape(...shape('x' => int, 'y' => bool), ...#v1) <: shape(...#v2, ...shape('x' => int, 'z' => string))`

* `#v1 ≤ shape('z' => string, 'x' => #f)` (upper bound on subtype's type variable)
* effective\_sub \= `merge(shape('x' => int, 'y' => bool), shape('z' => string, 'x' => #f))` \= `shape('x' => #f, 'y' => bool, 'z' => string)`
* Then: effective\_sub `<:` `shape(...#v2, ...shape('x' => int, 'z' => string))` (concrete definitive in supertype):
  * unknown: `nothing <: nothing`
  * `'x'`: `Req #f <: Req int` \-- `#f <: int`
  * `'z'`: `Req string <: Req string` \-- `string <: string`
  * `'y'`: only in effective subtype \-- `#v2 ≥ shape('y' => bool)`

**Req/Opt**: `shape(...shape('x' => int, 'y' => bool), ...#v1) <: shape(...#v2, ...shape(?'x' => int, 'z' => string))`

* `#v1 ≤ shape('z' => string, 'x' => #f)` (upper bound)
* effective\_sub \= `merge(shape('x' => int, 'y' => bool), shape('z' => string, 'x' => #f))` \= `shape('x' => #f, 'y' => bool, 'z' => string)`
* Then: effective\_sub `<:` `shape(...#v2, ...shape(?'x' => int, 'z' => string))` (concrete definitive in supertype):
  * `'x'`: `Req #f <: Opt int` \-- `#f <: int` (Req \<: Opt fine)
  * `'z'`: `Req string <: Req string` \-- `string <: string`
  * `'y'`: only in effective subtype \-- `#v2 ≥ shape('y' => bool)`

**Opt/Req**: `shape(...shape(?'x' => int, 'y' => bool), ...#v1) <: shape(...#v2, ...shape('x' => int, 'z' => string))`

- `#v1 ≤ shape('z' => string, 'x' => #f)` (upper bound)
- effective\_sub \= `merge(shape(?'x' => int, 'y' => bool), shape('z' => string, 'x' => #f))` \= `shape('x' => #f, 'y' => bool, 'z' => string)` (Req overrides Opt)
- Then: effective\_sub `<:` `shape(...#v2, ...shape('x' => int, 'z' => string))`:
  - `'x'`: `Req #f <: Req int` \-- `#f <: int`
  - `'z'`: `Req string <: Req string` \-- `string <: string`
  - `'y'`: only in effective subtype \-- `#v2 ≥ shape('y' => bool)`

**Opt/Opt**: `shape(...shape(?'x' => int, 'y' => bool), ...#v1) <: shape(...#v2, ...shape(?'x' => int, 'z' => string))`

- `#v1 ≤ shape('z' => string, 'x' => #f)` (upper bound)
- effective\_sub \= `merge(shape(?'x' => int, 'y' => bool), shape('z' => string, 'x' => #f))` \= `shape('x' => #f, 'y' => bool, 'z' => string)` (Req overrides Opt)
- Then: effective\_sub `<:` `shape(...#v2, ...shape(?'x' => int, 'z' => string))`:
  - `'x'`: `Req #f <: Opt int` \-- `#f <: int` (Req \<: Opt fine)
  - `'z'`: `Req string <: Req string` \-- `string <: string`
  - `'y'`: only in effective subtype \-- `#v2 ≥ shape('y' => bool)`

### Type variables in both, both rightmost

Subtype-side dual produces effective\_sub, then delegates to the supertype-side trick. Both fresh variables (`#f` on subtype side, `#g` on supertype side) are independent.

| Concrete subtype | Concrete supertype | Subtype fresh | Proposition | Overlapping 'x' | Test |
| :---- | :---- | :---- | :---- | :---- | :---- |
| Req | Req | Req \#f | `shape(...shape('x' => int, 'y' => bool), ...#v1) <: shape(...shape('x' => int, 'z' => string), ...#v2)` | `#f <: (int | #g)` | `both_rr_rr` |
| Req | Opt | Req \#f | `shape(...shape('x' => int, 'y' => bool), ...#v1) <: shape(...shape(?'x' => int, 'z' => string), ...#v2)` | `#f <: (int | #g)` | `both_rr_ro` |
| Opt | Req | Req \#f | `shape(...shape(?'x' => int, 'y' => bool), ...#v1) <: shape(...shape('x' => int, 'z' => string), ...#v2)` | `#f <: (int | #g)` | `both_rr_or` |
| Opt | Opt | Req \#f | `shape(...shape(?'x' => int, 'y' => bool), ...#v1) <: shape(...shape(?'x' => int, 'z' => string), ...#v2)` | `#f <: (int | #g)` | `both_rr_oo` |

**Req/Req**: `shape(...shape('x' => int, 'y' => bool), ...#v1) <: shape(...shape('x' => int, 'z' => string), ...#v2)`

* Subtype side: `#v1 ≤ shape('z' => string, 'x' => #f)` (upper bound \-- includes `'z'` from supertype's concrete splat)
* effective\_sub \= `merge(shape('x' => int, 'y' => bool), shape('z' => string, 'x' => #f))` \= `shape('x' => #f, 'y' => bool, 'z' => string)`
* Delegate: effective\_sub `<:` `shape(...shape('x' => int, 'z' => string), ...#v2)`
  * Supertype side: `#v2 ≥ shape('y' => bool, ?'x' => #g)` (lower bound \-- includes `'y'` from effective subtype)
  * effective\_super \= `merge(shape('x' => int, 'z' => string), shape('y' => bool, ?'x' => #g))` \= `shape('x' => (int | #g), 'y' => bool, 'z' => string)`
  * `'x'`: `Req #f <: Req (int | #g)` \-- `#f <: (int | #g)`
  * `'y'`: `Req bool <: Req bool` \-- `bool <: bool`
  * `'z'`: `Req string <: Req string` \-- `string <: string`

**Req/Opt**: `shape(...shape('x' => int, 'y' => bool), ...#v1) <: shape(...shape(?'x' => int, 'z' => string), ...#v2)`

* Subtype side: `#v1 ≤ shape('z' => string, 'x' => #f)` (upper bound)
* effective\_sub \= `merge(shape('x' => int, 'y' => bool), shape('z' => string, 'x' => #f))` \= `shape('x' => #f, 'y' => bool, 'z' => string)`
* Delegate: effective\_sub `<:` `shape(...shape(?'x' => int, 'z' => string), ...#v2)`
  * Supertype side: `#v2 ≥ shape('y' => bool, ?'x' => #g)` (lower bound)
  * effective\_super \= `merge(shape(?'x' => int, 'z' => string), shape('y' => bool, ?'x' => #g))` \= `shape(?'x' => (int | #g), 'y' => bool, 'z' => string)`
  * `'x'`: `Req #f <: Opt (int | #g)` \-- `#f <: (int | #g)` (Req \<: Opt fine)
  * `'y'`: `Req bool <: Req bool` \-- `bool <: bool`
  * `'z'`: `Req string <: Req string` \-- `string <: string`

**Opt/Req**: `shape(...shape(?'x' => int, 'y' => bool), ...#v1) <: shape(...shape('x' => int, 'z' => string), ...#v2)`

* Subtype side: `#v1 ≤ shape('z' => string, 'x' => #f)` (upper bound)
* effective\_sub \= `merge(shape(?'x' => int, 'y' => bool), shape('z' => string, 'x' => #f))` \= `shape('x' => #f, 'y' => bool, 'z' => string)` (Req overrides Opt)
* Delegate: effective\_sub `<:` `shape(...shape('x' => int, 'z' => string), ...#v2)`
  * Supertype side: `#v2 ≥ shape('y' => bool, ?'x' => #g)` (lower bound)
  * effective\_super \= `merge(shape('x' => int, 'z' => string), shape('y' => bool, ?'x' => #g))` \= `shape('x' => (int | #g), 'y' => bool, 'z' => string)`
  * `'x'`: `Req #f <: Req (int | #g)` \-- `#f <: (int | #g)`
  * `'y'`: `Req bool <: Req bool` \-- `bool <: bool`
  * `'z'`: `Req string <: Req string` \-- `string <: string`

**Opt/Opt**: `shape(...shape(?'x' => int, 'y' => bool), ...#v1) <: shape(...shape(?'x' => int, 'z' => string), ...#v2)`

* Subtype side: `#v1 ≤ shape('z' => string, 'x' => #f)` (upper bound)
* effective\_sub \= `merge(shape(?'x' => int, 'y' => bool), shape('z' => string, 'x' => #f))` \= `shape('x' => #f, 'y' => bool, 'z' => string)` (Req overrides Opt)
* Delegate: effective\_sub `<:` `shape(...shape(?'x' => int, 'z' => string), ...#v2)`
  * Supertype side: `#v2 ≥ shape('y' => bool, ?'x' => #g)` (lower bound)
  * effective\_super \= `merge(shape(?'x' => int, 'z' => string), shape('y' => bool, ?'x' => #g))` \= `shape(?'x' => (int | #g), 'y' => bool, 'z' => string)`
  * `'x'`: `Req #f <: Opt (int | #g)` \-- `#f <: (int | #g)` (Req \<: Opt fine)
  * `'y'`: `Req bool <: Req bool` \-- `bool <: bool`
  * `'z'`: `Req string <: Req string` \-- `string <: string`
