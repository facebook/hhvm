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
