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
avoid the documented intersection law bugs. This is a structural restriction,
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

## Case types intersected with nullable functions

[T288865283](https://www.internalfb.com/tasks/T288865283) tracks a completeness
bug exposed by the collection/container diff's intersection template, seed 365.
The checker rejects even this identity function with three `Typing[4110]` errors:

```hack
<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'case_types')>>
case type C = Awaitable<mixed>;
function test((C & ?(function(): int)) $x): (C & ?(function(): int)) { return $x; }
```

Inlining `Awaitable<mixed>`, replacing the case declaration with
`type C = Awaitable<mixed>`, adding `C as nonnull`, or removing the function's
nullable wrapper makes this pass.
`INTERSECTION_TYPE` conservatively excludes an exposed case type paired with
an exposed nullable function, in either order. Aliases, newtypes, and type
constants are followed on both operands and inside the nullable wrapper;
structural fields and case bodies are not treated as exposed function heads. Some case
bodies containing null or functions pass but are included in this narrow
syntactic exclusion. Ordinary `TYPE` generation retains these forms. Remove
this exception when the task's reproduction and controls pass.

The same task also covers an inhabited case-union identity failure:

```hack
<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'case_types')>>
case type C = int | bool;
case type F = int | ?bool;
function test((C & F) $x): (C & F) { return $x; }
```

The intersection-law guard also excludes two exposed case types with multiple
variants when one has a nullable variant. Exact definition bodies are retained
separately from subtype edges, since case bounds add reverse edges that are not
variants. Aliases, newtypes, type constants, and singleton case chains are
followed to the outer case union. The nullable variant can be exposed through
an alias, newtype, or type constant. The guard does not scan structural fields
or nested case variants. A singleton case wrapping a nullable type, and a case
union with a separate literal `null` variant, remain available. This is a
conservative syntax guard: explicit nonnull bounds and some primitive unions
also pass, but are not distinguished by the guard.

The task also rejects `C & null` identities for unbounded, nonnullable case
bodies. The intersection guard follows exact aliases and type constants to
this case/null pair, including `?null` on the null side. It retains case bodies
with an explicit nullable, null, mixed, or like variant, including nested case
and alias definitions, and case bounds known to be nonnull. Exact declared
case bounds are stored separately from subtype edges. A case wrapping the
null operand remains available; it is a passing control. Ordinary type
generation is unchanged.

## Verification and retained failures

The static verifier checks every diagnostic against `--hhstc-pattern`; one
matching expected error cannot hide an unrelated error or warning. Positive
`No errors` checks require a successful exit and no diagnostics. Explicit
negative-test alternatives remain in `test/milner/BUCK`. Timeouts and process failures fail verification.

Both verifiers accept `--timeout SECONDS` (default 180 per process) and
`--output-dir NEW_DIRECTORY`. The directory retains generated sources, a summary, and failure records with commands, exit status and
output. It must not already exist. Omitting the option preserves temporary-file
cleanup; HHBBC repositories remain temporary even when sources are retained.

From `hphp/hack`, set `MILNER_EXE`, `HHSTC_EXE`, and `HHVM_EXE` to absolute paths
to saved binaries before running these examples:

```sh
buck run @//mode/opt-clang //hphp/hack/test/milner:verify_well_typed -- \
  --milner-exe "$MILNER_EXE" --hhstc-exe "$HHSTC_EXE" \
  --template "$PWD/test/milner/templates/TypehintViolationException.php.template" \
  --hhstc-pattern 'No errors' --seed-range 1 1001 \
  --output-dir /tmp/milner-verify-static --timeout 180

buck run @//mode/opt-clang //hphp/hack/test/milner:verify_runtime -- \
  --milner-exe "$MILNER_EXE" --hhvm-exe "$HHVM_EXE" \
  --template "$PWD/test/milner/templates/TypehintViolationException.php.template" \
  --mode HHBBC --sample-size 200 --global-seed 42 \
  --output-dir /tmp/milner-verify-hhbbc --timeout 180
```

Static seed ranges exclude their upper bound. Runtime samples distinct seeds
before starting workers and refuses to overwrite generated files. A global seed
repeats that sample with the same harness; replaying an individual failure uses
its recorded generator seed with the same template and binary. Use
`--mode Sandbox` with a different output directory for the other runtime mode.

## Collections

Collection lengths are bounded; empty and populated vec, dict, and keyset values
are generated. Each element is an inhabitant of its declared type in the same
environment. The collection template checks reads, writes, iteration, and shape
operations with observable assertions.

## Builtin container abstractions

`Traversable`, `Container`, `KeyedTraversable`, `KeyedContainer`, `Iterator`,
`KeyedIterator`, and `vec_or_dict` have explicit covariance and subtype edges.
Arrays witness the container interfaces; iterator witnesses come from `Vector`
and `Map` iterators. Keys satisfy the arraykey bound, and vec witnesses require
an admitted int key type. Shared runtime representations are treated
conservatively when checking case-type disjointness.

## Stateful hierarchies, traits and interfaces

Class hierarchies share a typed constructor/member contract. Construction only
constructs the object; it does not run a test scenario. Expressions compose
syntax nodes for their constituent operations, and the typing model establishes
argument types, receiver relationships, lexical scope and required coeffects.
Assertions belong to small law templates.

`construct#1`, `read#1`, `write#1`, and `identity#1` are independent operations
from one generated hierarchy with payload `TYPE#1`. Reads and writes cross an
ancestor-typed function boundary. Static calls can select inherited members.
`hierarchy#1` composes compatible operations with bounded depth and a typed input;
it preserves that input without embedding any assertions. `dispatch#1` calls an
overridden constant-returning method through its ancestor; `DISPATCH#1` records
the concrete implementation's expected result.

`another#1` requests another inhabitant of the same type and environment. Literal
values vary, but two inhabitants need not differ, especially for singleton types.
The put/get law remains valid in either case; seeds with distinct values make a
dropped write observable.

`trait_read#1`, `interface_read#1`, and `interface_write#1` use the same
contract through trait and interface views. They are independent operations,
and the value-preserving calls also participate in expression composition.

## Nullable enum case returns in getter overrides

[T288899890](https://www.internalfb.com/tasks/T288899890) tracks a checker completeness
bug in concrete method overrides:

```hack
<?hh
<<file: __EnableUnstableFeatures('case_types')>>
enum E: int as int { A = 42; }
case type C = ?E;
class ParentClass { public function get()[]: C { return E::A; } }
class ChildClass extends ParentClass { <<__Override>> public function get()[]: C { return parent::get(); } }
```

Both signatures declare `C`, but return pessimisation produces `?int & ~C`
and the checker rejects the override with `Typing[4341]`. The runtime call
returns `42`. Removing the nullable wrapper, using a transparent alias, or
writing the equivalent multi-variant case `E | null` makes it pass.

For this payload shape, inherited getters remain inherited instead of emitting
a redundant override. The predicate follows actual singleton-case, alias,
local-newtype and like definitions to a nullable enum; subtype edges introduced
by case bounds do not count as definition variants. It stops at outer nullable
types, type constants and structural containers. Other getters still override,
and ordinary enum/case generation, setters, properties and dispatch remain.
