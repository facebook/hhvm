//// foo.php
<?hh
// package pkg2
<<file: __PackageOverride('pkg2')>>
type TShape = shape('x' => int);
type TInt = int;

//// bar.php
<?hh
// package pkg1

// Positions that look like enforceable type hints but are NOT runtime-
// enforced for the alias inside them: local `as` / `?as`, closure type
// internals, where clauses, refinement bodies, and class constants /
// type constants. None of these should fire the enforceable_type_alias
// check (each has its own dedicated rule, or no rule at all).

// Non-enforceable positions: should NOT produce enforceable_type_alias errors.
// Params are `mixed` so the as/?as assertions aren't redundant (Warn[12011]).
function non_enforceable_local(mixed $a, mixed $b): void {
  $y = $a as TShape;
  $z = $b ?as TShape;
}

// Closures: HHVM does not enforce the inner param/return types of a closure
// value at the boundary (the value itself is just a Closure object). Inner
// TShape/TInt inside a (function(..): ..) hint should NOT error.
function closure_param((function(TShape): TInt) $_): void {}
function closure_return(): (function(TShape): TInt) {
  return ($_) ==> 0;
}

// Where clauses: typechecker-only, not runtime-enforced. Should NOT produce
// enforceable_type_alias errors even with `T as TShape`.
function where_clause_shape<T>(T $_): void where T as TShape {}
function where_clause_int<T>(T $_): void where T as TInt {}

// Refinement type constants (`Foo with {type TC = TShape}`): the refinement
// is typechecker-only; HHVM only enforces the outer class. The nested type
// alias reference should NOT trigger enforceable_type_alias.
interface IHasTC {
  abstract const type TC;
}
function refinement_param(IHasTC with { type TC = TShape } $_): void {}
function refinement_return(): IHasTC with { type TC = TShape } {
  throw new Exception('unreachable');
}

// Class constants and type constants have their own package-boundary rules
// (distinct from enforceable_type_alias) and are not checked by this feature.
class WithConstants {
  const TShape SHAPE_CONST = shape('x' => 1);
  const type ALIAS_CONST = TShape;
}
