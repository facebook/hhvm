# Milner

Milner generates Hack programs from templates using an independent model of
typing and subtyping. The typechecker and runtime are test oracles, not filters
used to choose which generated programs to retain.

```
buck run @//mode/opt-clang //hphp/hack/src/milner:milner -- TEMPLATE --seed 42
```

`TYPE#1`, `SUBTYPE#1`, and `expr#1` request a type, a subtype, and an
inhabitant with the same integer identifier. Auxiliary definitions are appended
to the generated program. Templates supply the property under test and its legal
typing contexts.

Every generated value type has an inhabited subtype. Immediate inhabitance is a
pure structural check: selecting an inhabited subtype does not construct an
expression or consume randomness merely to decide whether a witness exists.

Expression construction carries the same generation context and nominal
environment through recursive witnesses.

Every identifier has one generated type and environment, including identifiers
that occur only in expressions. Their auxiliary declarations are retained.
Repeated placeholders share their replacement; longer numeric identifiers are
substituted first so `#1` cannot consume the prefix of `#10`.

`ALIAS_TYPE#1` constrains that identifier's type and subtype choices everywhere
to legal alias right-hand sides. It excludes direct type-constant references;
type constants nested in other type constructors remain available.

`INTERSECTION_TYPE#1` chooses types jointly with other marked identifiers to
avoid the documented intersection law bug. This is a structural restriction,
independent of checker execution.

## Intersection commutativity with like and nullable types

[T288868888](https://www.internalfb.com/tasks/T288868888): the checker rejects
this valid reordering with `Typing[4110]`:

```hack
<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'like_type_hints')>>
function takes((?string & ~int) $_): void {}
function test((~int & ?string) $x): void { takes($x); }
```

The same operand order, the reverse direction, and removing the like operator
pass. The `INTERSECTION_TYPE` context conservatively excludes a like head paired
with a nullable form. It follows aliases, newtypes and type constants; a case
type can hide a nullable union. Required tuple elements and shape fields are
followed as well, but a case type wrapping the like head does not
trigger the bug. Ordinary `TYPE` generation keeps these forms available.

The union-introduction template also constructs typed witnesses before widening
them into the union. This avoids the existing nullable case-type closure
contextual-coercion bug, [T201523298](https://www.internalfb.com/tasks/T201523298),
while retaining function-bearing case types.
