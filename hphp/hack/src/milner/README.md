# Milner

Milner generates well-typed Hack programs by construction, using an independent
model of typing and subtyping. This exposes typechecker completeness bugs:
valid programs that the checker rejects. Programs accepted by the checker then
exercise HHVM, including the JIT and HHBBC, to find runtime bugs.

The [announcement post](https://fb.workplace.com/groups/hacklang.eng/permalink/26678742515081011/)
describes the motivation and examples of bugs Milner has found.

## Design and templates

Generation is type-directed: Milner chooses types, generates their supporting
declarations, and constructs expressions that inhabit them. Adding the laws for
a language feature lets it combine with other features throughout generation.
The typechecker tests this independent model; it never decides which generated
programs to keep.

[Templates](../../test/milner/templates/) encode high-level properties, such as
subtype substitution, method variance, union/intersection laws, and successful
execution across runtime type boundaries. They supply the program skeleton:

| Placeholder | Meaning |
| --- | --- |
| `TYPE#N` | A generated type. |
| `SUBTYPE#N` | A model-justified subtype of `TYPE#N`. |
| `expr#N` | An expression inhabiting `TYPE#N`. |

Repeated occurrences receive the same substitution. The integer connects the
three kinds of placeholder; different integers introduce separate choices.
`TYPE#N` need not appear explicitly for the other placeholders to use it.
`expr#N` does not necessarily inhabit the independently chosen `SUBTYPE#N`.

For example, this template states that a subtype's value can be returned as
its supertype:

```hack
<?hh
function widen(SUBTYPE#1 $value): TYPE#1 {
  return $value;
}
```

Most templates expect acceptance. Others encode properties requiring rejection,
such as overlapping case types; their expected diagnostics are specified in
[the test definitions](../../test/milner/BUCK). Extend the shared generators for
new language constructs, and add templates for new properties.

## Randomness

Randomness explores lawful choices of types, subtype relations, expressions,
and declaration and call forms. Keep choices broad and compositional so feature
interactions arise without enumerating them by hand. Size budgets bound
recursive generation, and size distributions favor small programs while
retaining larger cases.

A template and seed reproduce a program for a fixed generator revision. Changes
to generation can change that program, so preserve its source and revision.
Known bugs need narrow structural exceptions, never seed blacklists or retries
based on whether the checker accepts the output.

## Bug detection pipeline

1. **Typecheck generated programs.** Unexpected rejection of a positive
   template is a candidate completeness bug. Check the generator's reasoning
   too: its model can be wrong.
2. **Execute accepted programs.** Run HHVM directly in Sandbox mode and in repo
   mode after whole-program HHBBC optimization. The HHBBC harness enables the
   JIT and repeats execution. Reproduce with the JIT disabled to distinguish
   interpreter behavior from JIT behavior.
3. **Check the property.** Unexpected diagnostics, compilation failures,
   crashes, timeouts, and failed assertions are failures. Runtime failures in
   accepted programs can reveal typechecker soundness or compiler/runtime bugs.

A timeout may indicate that a generated program exposes a performance bug or
an infinite loop in the typechecker, runtime, compiler, or optimizer. Investigate
these failures and try to reproduce them with the retained program and command.

Runtime verification checks process success and generated assertions; it does
not compare results between modes. Add assertions for properties requiring
exact values or effects.

## Running

From `hphp/hack`, build the generator and standalone checker:

```sh
buck build @//mode/opt-clang --show-full-output \
  //hphp/hack/src/milner:milner \
  //hphp/hack/src:hh_single_type_check
```

Set `MILNER_EXE` and `HHSTC_EXE` to the absolute paths printed by the build.
Set `HHVM_EXE` to the binary to test, for example `/usr/local/hphpi/bin/hhvm`.

Generate and typecheck one program:

```sh
TEMPLATE="$PWD/test/milner/templates/TypehintViolationException.php.template"
"$MILNER_EXE" "$TEMPLATE" --seed 42 --destination /tmp/milner-42.php
"$HHSTC_EXE" /tmp/milner-42.php
```

Check seeds 1 through 100:

```sh
buck run @//mode/opt-clang //hphp/hack/test/milner:verify_well_typed -- \
  --milner-exe "$MILNER_EXE" --hhstc-exe "$HHSTC_EXE" \
  --template "$TEMPLATE" --hhstc-pattern 'No errors' --seed-range 1 101 \
  --output-dir /tmp/milner-static
```

Run the same reproducible runtime sample in both modes:

```sh
for mode in Sandbox HHBBC; do
  buck run @//mode/opt-clang //hphp/hack/test/milner:verify_runtime -- \
    --milner-exe "$MILNER_EXE" --hhvm-exe "$HHVM_EXE" \
    --template "$TEMPLATE" --sample-size 100 --global-seed 42 \
    --mode "$mode" --output-dir "/tmp/milner-$mode" || exit
done
```

The runtime harness samples its own generator seeds and does not typecheck.
Its sample differs from the static range above: typecheck retained runtime
inputs before attributing failures to the runtime. Sandbox uses HHVM's normal
configuration; for a minimized single-file repro, force interpreter execution:

```sh
"$HHVM_EXE" -vHack.Lang.AllowUnstableFeatures=1 -vEval.Jit=0 repro.php
```

Both verifiers accept `--timeout SECONDS` (180 by default). Each
`--output-dir` must be new; it retains sources, a summary, and failure records
with commands and diagnostics. The runtime harness handles multifile programs;
compiled HHBBC repositories remain temporary. Replay generation with the
recorded generator seed, template, and binary. Retained single-file repros can
be checked and run directly.

## Handling bugs

Reduce the failing program and file a task with the repro, expected and actual
behavior, exact commands, tool revisions, and failing execution modes. Fix
generator mistakes rather than treating them as language bugs.

For a confirmed implementation bug, add the smallest structural exception that
prevents Milner from generating it again. Reference the task in a comment beside
the exception and preserve nearby lawful cases so generation can find other
bugs. Keep detailed repros and investigation history in the task. Remove the
exception once the fix passes the repro and relevant controls.
