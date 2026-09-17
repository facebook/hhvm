# Milner

Milner generates Hack programs from templates using an independent model of
typing and subtyping. The typechecker and runtime are test oracles, not filters
used to choose which generated programs to retain.

```sh
buck run @//mode/opt-clang //hphp/hack/src/milner:milner -- TEMPLATE --seed 42
```

## Template interface

The interface has three placeholders:

| Placeholder | Meaning |
| --- | --- |
| `TYPE#1` | A generated value type. |
| `SUBTYPE#1` | A model-justified subtype of that type. |
| `expr#1` | An expression inhabiting that type. |

Each integer identifier has one type and one declaration environment, including
identifiers occurring only in expressions. Repeated placeholders have identical
substitutions; identifier boundaries distinguish `#1` from `#10`. The expression
inhabits `TYPE#1`, not necessarily the independently selected `SUBTYPE#1`.

The ten templates state general properties: runtime type boundaries, subtype
substitution, method variance, union/intersection laws, and case-type rejection
or termination. New constructs belong in type and expression generation, so the
same templates can combine them without new placeholder vocabulary.

Generation constraints come from context. A type placeholder on the right-hand
side of a `type`, `newtype`, or `case type` declaration excludes direct
type-constant references; nested references remain available. Placeholders
joined by `&` receive the intersection compatibility checks documented below.
These restrictions use template syntax, independently of checker execution.

## Generation model

Types, subtype edges, exact nominal definitions, and construction witnesses share
an environment. Exact alias and case bodies are distinct from subtype edges:
case bounds can introduce reverse edges without adding declaration variants.
Every generated value type has an inhabited subtype. Immediate inhabitance is a
structural check that neither builds an expression nor consumes randomness.

`Type.inhabitant_of` builds a scoped expression and retains any declarations it
introduces. Its bounded grammar combines calls, captures, inout and variadic
arguments, control flow, mutation, exception handling, and asynchronous tasks
with operations justified by the generated nominal environment. The same shared
size budget bounds recursive expression and task composition. Optional tuple
and nullsafe projections coalesce boxes before extracting their payload, keeping
`Awaitable` payloads legal. Reduced-capability callbacks capture an already
evaluated value.

These operations preserve the result type; they may change the value. A generic
write can select another inhabitant, an enum lookup can return another member,
and a dispatch call can select an overridden implementation. The original
positive templates check typing laws and successful runtime execution. They do
not assert exact returned values, dispatch results, or effect counts.

The type model includes bounded collections, optional tuples, container
interfaces, class hierarchies, traits, generic variance and bounds, dependent
type constants and refinements, callable contexts, enum members and labels, and
class names/pointers. Collection elements and constructor arguments are generated
in the same environment. Closed tuple witnesses omit only an optional suffix;
`void` and `nothing` are function results, never standalone value types or
required fields. Function subtyping can widen parameters, narrow value results,
and decrease required capabilities. Lambdas carry explicit contexts, including
`[defaults]`, rather than inheriting an accidental enclosing context.

Protocol fixtures are internal declarations. XHP uses nominal payload boxes;
string payloads also admit child projections. Expression-tree composition uses
`mixed` payloads. Composed enum lookup/unwrap operations use primitive payloads,
while the ordinary type grammar retains richer enum members and labels. Class
identity types remain excluded from reified positions.

Eligible entrypoint programs may use a module layout with separate module,
definition, and main files. Internal helper calls cross files within the same
module. Programs with local newtypes stay in one file so their representations
remain visible. This layout is selected by generation, without a dedicated
template.
Each virtual file has its own `<?hh` header and uses `//// relative/path.php`
delimiters. The generated entrypoint loads declarations before invoking the
original entrypoint.

## Verification and retained failures

The static verifier checks every diagnostic against `--hhstc-pattern`; one
matching expected error cannot hide an unrelated error or warning. Positive
`No errors` checks require a successful exit and no diagnostics. Explicit
negative and recursive acceptance policies are in `test/milner/BUCK`. Timeouts
and process failures fail verification.

Both verifiers accept `--timeout SECONDS` (default 180 per process) and
`--output-dir NEW_DIRECTORY`. The directory must not already exist. It retains
generated sources, a summary, and failure records containing commands, exit
status, and output. Without it, temporary files are cleaned up. HHBBC repositories
remain temporary even when sources are retained.

From `hphp/hack`, set `MILNER_EXE`, `HHSTC_EXE`, and `HHVM_EXE` to absolute paths
to saved binaries:

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
repeats the sample with the same harness; replay an individual failure with its
recorded generator seed, template, and binary. Use `--mode Sandbox` with another
output directory to exercise that runtime mode separately.

The checker receives a multifile bundle intact. Runtime checks split validated,
unique relative paths into a fresh directory, select `main.php` or the unique
entrypoint, and pass every physical file to HHBBC. Retained output includes the
split files. Runtime success alone does not compare Sandbox and HHBBC results.

## Known completeness and runtime constraints

The following structural restrictions keep known bugs out of lawful positive
generation. They do not suppress diagnostics or discard checker-rejected seeds.
Remove a restriction only after its reproduction and passing controls support
doing so.

### Intersection laws

[T288868888](https://www.internalfb.com/tasks/T288868888): reordering
`~int & ?string` to `?string & ~int` fails with `Typing[4110]`. The same order,
reverse direction, and removal of the like operator pass. Intersection operands
therefore exclude a like head paired with a nullable form. The guard follows
aliases, newtypes, type constants, tuple elements, shape fields, and nullable
unions hidden by cases. A case wrapping the like head does not trigger it.
Ordinary type generation retains these forms.

[T288865283](https://www.internalfb.com/tasks/T288865283) covers several related
identity and projection failures:

```hack
<?hh
<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'case_types')>>
case type C = Awaitable<mixed>;
function test((C & ?(function(): int)) $x): (C & ?(function(): int)) {
  return $x;
}
```

Inlining the case body, making it a transparent alias, adding `C as nonnull`,
or removing the function's nullable wrapper passes. The guard excludes exposed
case types paired with exposed nullable functions in either order. It follows
aliases, newtypes, and type constants, including inside the nullable wrapper;
structural fields and case bodies are not exposed function heads. Some passing
case bodies remain conservatively excluded.

The task also covers `C = int | bool`, `F = int | ?bool`, and the identity at
`C & F`. The guard excludes two exposed multi-variant cases when one has a
nullable variant. It follows exact aliases, newtypes, type constants, dependent
constants, and singleton case chains, without scanning structural fields or
nested case variants. A singleton case wrapping a nullable type, or a separate
literal `null` variant, remains available. Some passing primitive unions and
nonnull-bounded cases are conservatively excluded.

An unbounded nonnullable case intersected with `null` or `?null` can fail too.
The guard follows exact definitions, retaining cases with known nullable, null,
`mixed`, or like variants and cases with known nonnull bounds. A case wrapping
the null operand remains available as a passing control.

Opaque labels also fail against nullable functions: an identity at
`HH\EnumClass\Label<E, int> & ?(function(): int)` cannot prove the checker's
`Label <: nonnull | (Label & null)` partition. This pair is excluded through
aliases, newtypes, and concrete type constants. Nullable primitive, class,
tuple, and Awaitable operands remain available.

The same partition failure affects `HH\MemberOf<E, T>` when the payload exposes
`mixed`, an unbounded case, a label, or certain nullable bounds; bare `null` and
null/function-only cases can expose it. The guard follows exact alias, newtype,
type-constant, dependent, and nested-member payload definitions, but not
structural fields. Like wrappers retain the nullable-bound hazard. Nonnullable
payloads, null-only nullable payloads, matching emitted nullable-function
signatures, and suitable declared case bounds remain available. The guard does
not model full function subtyping, so some passing signatures are omitted.
Outer nullable members and ordinary enum generation remain available.

[T288868918](https://www.internalfb.com/tasks/T288868918): projecting
`classname<D> & class<C>` to `class<C>`, with `D extends C`, fails because
intersection normalization produces `classname<C & D>` and loses the pointer
constraint. `D::class` inhabits the intersection; pointer-only and same-class
controls pass. The guard follows name/pointer heads through transparent wrappers,
case branches, tuple slots, and shape fields. It also follows vec elements when
the other operand exposes a tuple. Vec/vec, shape/dict, function, and generic
container controls pass and do not receive that extra traversal.

### Contextual returns and calls

[T201523298](https://www.internalfb.com/tasks/T201523298): nullable unions can
reject directly contextualized case-type closures. The union-introduction
template first constructs explicitly typed witnesses, then widens their results
into the union, retaining function-bearing case types.

[T288899890](https://www.internalfb.com/tasks/T288899890): a redundant getter
override returning a singleton case around a nullable enum can fail with
`Typing[4341]`. For `enum E: int as int` and `case type C = ?E`, both methods
can declare `C` while return pessimisation produces `?int & ~C`. Runtime returns
the enum value; a transparent alias, a nonnullable case, or `E | null` passes.

Such getters remain inherited. The predicate follows singleton-case, alias,
local-newtype, and like definitions to a nullable enum, ignoring reverse case
bound edges. It stops at outer nullable types, type constants, and structural
containers. Other getter overrides, setters, properties, and dispatch remain.

A singleton case around `?classname<C>` also rejects identical getter overrides
with `Typing[4341]`, with or without a generic parent. A transparent alias, a
nonnullable classname case, and a nullable `class<C>` case pass. The same
inherited-getter restriction covers this case; class names remain generated.

[T288960552](https://www.internalfb.com/tasks/T288960552): an immediate variadic
lambda called with two arrays followed by an iterator can acquire an incompatible
`dynamic` constraint despite its legal `mixed ...$xs` parameter. Binding the
callee or unpacking the arguments passes. Callable application uses `...vec[...]`
for multi-element independently generated tails; zero/one-tail calls remain.

### Enum initialization and consumption

[T288868921](https://www.internalfb.com/tasks/T288868921):
`Values::valueOf($label)` may require `dynamic` when a
`Label<Values, nonnull>` is consumed as `nonnull`. Explicit
`<Values, nonnull>` arguments or an intermediate local pass. Generated lookups
supply the exact owner and payload arguments.

[T288868928](https://www.internalfb.com/tasks/T288868928): coercing
`MemberOf<Outer, MemberOf<Inner, mixed>>` to `MemberOf<Inner, mixed>` misses the
outer upper bound while comparing same-name newtypes. Generated consumption uses
a checked helper `unwrap<T>(MemberOf<Outer, T> $x)[]: T { return $x; }`,
instantiated at the exact payload type.

[T288868934](https://www.internalfb.com/tasks/T288868934): copying another enum
member whose value is a label can typecheck but abort interpreter/JIT dynamic
initialization in `Class::clsCnsGet`:

```hack
<?hh
enum class Values: int { int A = 42; }
enum class Labels: mixed { HH\EnumClass\Label<Values, int> A = Values#A; }
enum class Nested: mixed { HH\EnumClass\Label<Values, int> A = Labels::A; }
<<__EntryPoint>>
function main(): void { $value = Nested::A; }
```

Literal-label initialization and object-box containment pass, as does this
indirect example after HHBBC optimization. Dynamic constant validation rejects
`KindOfEnumClassLabel`, including inside arrays. Enum initializer contexts
therefore exclude labels before type, subtype, and witness generation. The
restriction propagates conservatively through functions and objects even when
those wrappers are safe. Ordinary value contexts retain labels; the context is
not a validator for an arbitrary previously generated type.

[T288894213](https://www.internalfb.com/tasks/T288894213): repeated
`HH\MemberOf` upper-bound expansion is mistaken for a cycle even when nesting is
finite. The alternative
`HH\MemberOf<Outer, HH\MemberOf<Inner, int>>` can be widened to `mixed`, causing
its case union with `Awaitable<mixed>` to fail `Typing[4475]`. Same-owner nesting,
nullable wrappers, aliases, local newtypes, type constants, and other cases can
reproduce this; a single-variant case passes.

Disjointness comparison treats a member as `mixed` when its payload exposes
another member through these wrappers. This prevents competing case alternatives
without removing ordinary nested members or singleton cases. Traversal stops at
tuples, shapes, arrays, containers, classes, functions, and Awaitable because
their outer runtime tags hide the payload. Newtypes follow their actual bodies:
a nested member only in an upper bound does not trigger the guard when the body
is `null`. This is distinct from the nested-member coercion bug above.

### XHP attribute hints

[T288868908](https://www.internalfb.com/tasks/T288868908): direct structural XHP
attribute hints typecheck but fail emission with "There are no other possible
xhp attribute hints". `MilnerPayload<T>` keeps the attribute nominal and the
payload general. It also handles the separate restrictions on nullable required
attributes and direct type-constant hints.
