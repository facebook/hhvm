# Conditional Case Types

A variant of a [case type](/hack/types/case-types) can carry a `where` clause describing conditions on type parameters. The variant then only applies to instantiations that satisfy those constraints, and when you narrow a value back down to a single variant, the type checker can use the constraints to recover information about the type parameters.

## Basic Syntax

```
case type Name<T1, T2> =
  | Variant1 where Constraint, Constraint
  | Variant2;
```

A `where` clause is a comma-separated list of constraints, each written as `Type as Type`, `Type super Type`, or `Type = Type`. Variants with and without `where` clauses can be mixed freely.

A variant's `where` clause is assumed only while that variant itself is being checked. Where clauses do not affect disjointness.

## Which Instantiations Accept a Variant

A value belongs to `Name<T1, T2>` if it belongs to one of the variants *and* that variant's constraints hold for those type arguments.

```hack
case type CT<T> =
  | num where T super arraykey
  | bool where T = bool
  | string where T as num;

function satisfies_super(float $x): CT<arraykey> {
  return $x; // OK: arraykey is a supertype of arraykey
}

function violates_super(float $x): CT<int> {
  return $x; // ERROR: int is not a supertype of arraykey
}

function satisfies_eq(bool $x): CT<bool> {
  return $x; // OK: T is exactly bool
}

function satisfies_as(string $x): CT<int> {
  return $x; // OK: int is a subtype of num
}
```

## Variance

In `where L as R`, `L` appears in a contravariant position and `R` in a covariant one. `where L super R` is the mirror image, and `where L = R` makes both sides invariant.

So a covariant parameter `+T` may appear on the left of a `super` constraint, and a contravariant `-T` may appear on the left of an `as` constraint. An invariant parameter may appear anywhere.

```hack
interface IFoo {}
interface IBar {}

case type Covariant<+T> =
  | int where T super IFoo
  | string;

case type Contravariant<-T> =
  | int where T as IBar
  | string;
```

## Recovering Type Parameters

During decomposition using `is` checks, the typechecker can infer constraints.

```hack
final class MyInt {}
final class MyString {}
final class MyBool {}

case type LiftableTo<+T> =
  | int where T super MyInt
  | string where T super MyString
  | bool where T super MyBool;

function lift<T>(LiftableTo<T> $liftable): T {
  if ($liftable is int) {
    return new MyInt(); // T super MyInt
  } else if ($liftable is string) {
    return new MyString(); // T super MyString
  } else {
    return new MyBool(); // T super MyBool
  }
}
```

## When Refinement Applies

The type checker only assumes a variant's constraints when the check isolates that single variant. A check that could match more than one variant tells it nothing.

```hack
final class MyInt {}
final class MyString {}

case type IntOrString<+T> =
  | int where T super MyInt
  | string where T super MyString;

function ambiguous<T>(IntOrString<T> $x): T {
  if ($x is arraykey) {
    return new MyInt(); // ERROR (no inference is made)
  }
  throw new Exception();
}
```

## Runtime

- `where` clauses are erased at runtime, and do not appear in type structures.
