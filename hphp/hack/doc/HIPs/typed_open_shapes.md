**Status**: Draft, prototype implementation behind `typed_open_shapes` typechecker option
**Audience**: Hack Improvement Proposal committee
**Author**: Andrew Kennedy

# HIP: Typed Open Shapes

## Summary

Today, an *open* shape — written `shape('a' => int, ...)` — says nothing about
the type of the fields not listed by name. The compiler treats them as
`mixed`. This proposal generalises the `...` syntax to allow an explicit type:

```hack
// Existing: untyped open shape — unknown fields have type mixed
type ConfigU = shape('a' => int, ...);

// New: typed open shape — every unknown field must be a string
type ConfigT = shape('a' => int, string...);

function f(ConfigT $c): void {
  $x = $c['a'];          // : int
  $y = Shapes::idx($c, 'anything');   // : ?string (because every unknown field is a string), with null default
  $z = Shapes::idx($c, 'anything', 'default'); // : string because there is a string default
}
```

Plain `...` continues to mean "unknown fields are `mixed`" and is therefore
equivalent to writing `mixed...` explicitly.

## Motivation

### Goal

Hack already lets users say "this shape has a known field of type `X`". It
also lets users say "this shape may have other fields, of any type". What it
cannot currently express is the very common middle ground: "this shape has a
known field of type `X`, and may have other fields, all of which have type
`Y`". That is what this proposal adds.

Concretely, this comes up whenever a shape models a "metadata + payload"
record:

```hack
// A logging event: one well-known "level" field plus arbitrary structured
// data, where the data is always JSON-serialisable.
type LogEvent = shape('level' => LogLevel, mixed...);

// A request envelope: a well-known method, plus header/value pairs that
// must all be strings.
type Request = shape('method' => string, string...);

// A sparse number map: one canonical "total" plus per-bucket counts.
type Histogram = shape('total' => int, int...);
```

Without typed open shapes, the only way to express any of the above is to
weaken the unknown-field type to `mixed` and then narrow at every use site
with `is`/`as` or `Shapes::idx`-and-cast. That is verbose, error-prone, and
hides the developer's intent.

## User experience

Authoring is a simple extension of existing syntax: `...` becomes
`T...` to bind a type to unknown fields. This syntax is already used for similar
purpose in variadic function types, and open tuple types. Everything else — field syntax,
optional-field syntax, `Shapes::` helpers — stays as it is.

```hack
// Function & method hints
function takes(shape('id' => int, string...) $req): void {}

// Type aliases
type TypedOpen = shape('a' => int, ?'b' => bool, vec<int>...);

// Class type constants
class C {
  const type TX = shape('a' => int, ?'b' => string, MyEnum...);
}

// Nested in generics
function batch(vec<shape('a' => int, string...)> $v): void {}
```

We call the type of the open part of the shape a *variadic* type,
in common with variadic parameters and the variadic component of open tuple types.
### Equivalence: `...` and `mixed...`

These two forms are intentionally indistinguishable at every observable
boundary:

```hack
// All three describe the same type
type A = shape('x' => int, ...);
type B = shape('x' => int, mixed...);
type C = shape('x' => int, \HH\mixed...);
```

This invariant lets us add the syntax without breaking any existing program
that uses `...`. It is enforced both at the typechecker level (subtyping
treats them identically) and at the HackC level (the bytecode emitter
collapses `mixed...` and `...` to the same type-structure shape — no
`variadic_type` field is written).

Note also that `shape('x' => int, nothing...)` is equivalent to the *closed* shape type `shape('x' => int)`.

### IDE experience

Hover/completion shows the typed-open form verbatim — `shape('a' => int,
string...)` — when the source uses it, and the legacy `shape('a' => int,
...)` form otherwise.

### Errors

When the typechecker is run with the option `typed_open_shapes=false`, naming raises:

```
Naming[2132] Typed open shapes (e.g. shape('x' => int, T...)) are not enabled.
Use plain ... for unknown fields.
```

### Subtyping

Subtyping on shapes follows the principle: fields in the subtype should be at least as defined as fields in the supertype, and the type should be at least as specific.
In particular:

* If a field `'a' => t` or `?'a' => t` appears in the subtype, but not in the supertype, then the supertype should have a variadic type `u...` where `t <: u`. (The variadic type may be `mixed`, or left unspecified and default to `mixed`.)
* If the subtype is open with a variadic type `t`, then the supertype must be open, with variadic type `u` such that `t <: u`.

### Indexing

As with optional fields, and existing open shapes, fields should be accessed using `Shapes::idx($x, 'a')` (returning `null` if not present), or `Shapes::idx($x, 'a', $def)`, returning `$def` if not present.

### Type structure representation

For a `TypeStructure` that represents a shape type with variadic component, the optional `variadic_type` field
is set to a `TypeStructure` representation of the variadic type. This field is *not* set if the variadic type is `mixed` i.e. it's a normal open shape type.
The boolean `allows_unknown_fields` field is set to `true`.

## Rollout

Initially, the feature is enabled in the typechecker only if the `.hhconfig` flag is set `typed_open_shapes=true`. Once the changes land in HHVM (for HackC and type structures), the feature can be safely enabled.
