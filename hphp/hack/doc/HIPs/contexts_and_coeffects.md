### Feature Name: Contexts and CoEffects

### Start Date: July 14, 2020

### Status: Candidate

# Summary:

A generalized system for the description and enforcement of permissions and restrictions of a context.

This HIP presents feature that overlays a coeffect system into the Hack type system through lightweight context annotations that logically map into a set of capabilities. These capabilities establish both the calling conventions (i.e., which functions/method may call which other functions/methods) as well as the operations permitted within the present context.

“Contexts are like onions.”

# Feature motivation:

Several important in-progress language features require us to alter behavior dependent on the executing context. Examples of this include:

* **Context Implicit Purpose Policies (CIPP)**: Inside of a CIPP context, there are stricter rules about how data can be handled in order to ensure their purposes are maintained.
* **Purity/Reactivity:** In a pure/reactive context, the programmer cannot access global data nor mutate external references in order to achieve enforced determinism.

In order to support execution contexts with different permissions/restrictions, we propose adding **Contextual Effects (Coeffects)** to the Hack type system. Coeffects are expressive enough to support generalized execution scopes while also lending themselves nicely to syntactic sugar that will make them easy to use and require minimal changes to typechecker internals.

# Definitions

In the following document we will use a few terms either not previously defined within the context of Hack or else potentially nonspecific. For the sake of clarity, we will first have a list of definitions for reference throughout the rest of the document.

Capability: a permission or description of a permission. For example, one might consider the ability to do `io` or access globals as capabilities.

Context: a higher level representation of a set of capabilities. A function may be comprised of one or more contexts which represent the set union of the underlying capabilities.

Example context: for the purpose for the rest of the defintions, a context created solely for the purpose of this document rather than for intended release.

The Pure Context: A simple way of saying the context representing the empty list of capabilities `{}`.

`rand`: An example context representing the capabilities required to do random number generation. Refers to the set of capabilities `{Rand}`.

`io`: An example context representing the capabilities required to do IO. Refers to the set of capabilities `{IO}`.

`throws<T>`: A parameterized example context representing the capabilities required to throw exception type `T` or any children of `T`. Refers to the set of capabilities `{Throws<T>}`. One may consider that the type parameter of this context is covariant.

`defaults`: The context representing the set of capabilities present in a function prior to the introduction of this feature. Refers to the set of capabilities `{Throws<mixed>, IO, Rand}`.

Higher order functions: A function accepting, as one of its arguments, a value containing a function-type such as a function pointer or closure.

# User Experience - Syntax

Note that all of the below syntax is still very much up for bikeshedding and may not be finalized.

The specification for the syntax of a function including co-effects is as follows, with the list of co-effects itself being optional.

```
function function_name<list_of_generics>(
  parameter_type_hint variable_name,
  ...
)[list_of_contexts] : return_type_hint where_clauses {
  function_body
}
```

## Declaring Contexts and Capabilities

This is done natively within the typechecker and the runtime. More information on this is provided under those sections.

## Basic Declarations and Closures

A function or method may optionally choose to list one or more contexts:

```
function no_contexts(): void {...}
function one_context()[C]: void {...}
function many_context()[C1, C2, ..., Cn]: void {...}
```

Additionally, the context list may appear in a function type:

```
function has_fn_args(
  (function (): void) $no_list,
  (function ()[io, rand]: void) $list,
  (function ()[]: void) $empty_list,
): void {...}
```

As with standard functions, closures may optionally choose to list one or more contexts. Note that the outer function may or may not have its own context list. Lambdas wishing to specify a list of contexts must include a (possibly empty) parenthesized argument list.

```
function some_function(): void {
  $no_list = () ==> {...};
  $single = ()[C] ==> {...};
  $multiple = ()[C1, C2, ..., Cn] ==> {...};
  $with_types = ()[C]: void ==> {...};
  // legacy functions work too
  $legacy = function()[C]: void {};

  // does not parse
  $x[C] ==> {}
}
```

## Higher Order Functions With Dependent Contexts

One may define a higher order function whose context depends on the dynamic context of one or more passed in function arguments.

```
function has_dependent_fn_arg(
  (function()[_]: void) $f,
)[rand, ctx $f]: void {... $f(); ...}

function has_dependent_fn_args(
  (function()[_]: void) $f,
  (function()[_]: void) $f2,
)[rand, ctx $f, ctx $f2]: void {... $f(); ... $f2(); ...}
```

One may reference the dependent context of a function argument in later arguments as well as in the return type.

```
function has_double_dependent_fn_arg(
  (function()[_]: void) $f1,
  (function()[ctx $f1]: void) $f2,
)[rand, ctx $f1]: void {$f1(); $f2(); }

function has_dependent_return(
  (function()[_]: void) $f,
)[rand, ctx $f]: (function()[ctx $f]: void) {
  $f();
  return $f;
}
```

Attempting to use the dependent context of an argument before it is defined will result in an error about using an undefined variable.

Note that the special `_` placeholder context may only be used on function arguments.

```
// The following are all disallowed
type A = (function()[_]: void);
newtype A = (function()[_]: void);

Class Example {
  public (function()[_]: void) $f;
  public static (function()[_]: void) $f2;
}
function example(): (function()[_]: void) {}
```

## Constants

In addition to standard and type constants, classes may define context constants:

```
class WithConstant {
  const ctx C = [io];
  ...
}
```

Context constants may be abstract, and possibly have a default. If they are abstract, they may additionally contain one or both of the as and super constraints.

```
interface WithAbstractConstants<T> {
  abstract const ctx C1; // bare abstract
  abstract const ctx C2 = [io]; // abstract with default
  abstract const ctx C3 as [io, rand]; // abstract with bound
  abstract const ctx C4 super [io, rand] = [io]; // abstract with bound and default

  // Disallowed: Concrete with bound.
  const ctx C5 super [io, rand] = [io];
}
```

Context constants are accessed off of function arguments in a similar manner to function-type arguments. The same restrictions about use-before-define apply.

```
function type_const(SomeClassWithConstant $t)[$t::C]: void { $t->f(); }
```

Context constants are accessed off `this` or a specific type directly within the contexts list:

```
public function access_directly()[this::C1, MySpecificType::C2]: T {...}
```

Context constants may not be referenced off of a dependent/nested type. Said another way, context constants may only have the form `$arg::C`, not `$arg::T::C`, etc. It is possible we will relax this restriction in a future version.

```
interface IHasCtx {
  abstract ctx C;
}

interface IHasConst {
  abstract const type TC as IHasCtx;
}

// Disallowed: nested type acces
function type_const(IHasConst $t)[$t::TC::C]: void {}

abstract class MyClass implements IHasConst {
  // Disallowed: also applies to dependent types of this
  public  function type_const()[this::TC::C]: void{}
}
```

For the sake of simplicity, `this` must be used in lieu of `$this` within the context list.

## Additional Dependent Contexts Information

Dependent contexts may be accessed off of nullable parameters. If the dynamic value of the parameter is `null`, then the contexts list will be empty.

```
function type_const(
  ?SomeClassWithConstant $t,
  ?(function()[_]: void) $f,
)[$t::C, ctx $f]: void {
  $t?->foo();
  if ($f is nonnull) {
    $f();
  }
}
```

Parameters used for accessing a dependent context may not be reassigned within the function body.

```
function type_const(SomeClassWithConstant $t)[$t::C]: void {
  // disallowed
  $t = get_some_other_value();
}

function has_dependent_fn_arg((function()[_]: void) $f)[ctx $f]: void {
  // disallowed
  $f = get_some_other_value();
}
```

Dependent contexts may not be referenced within the body of a function. This restriction may be relaxed in a future version.

```
function f(
  (function()[_]: void $f,
  SomeClassWithConstant $t,
)[rand, ctx $f, $t::C]: void {
  (()[ctx $f] ==> 1)();    // Disallowed
  (()[$t::C] ==> 1)();    // Disallowed
  (()[rand] ==> 1)(); // Allowed, not a dependent context
  (()[] ==> 1)();     // Allowed
  (() ==> 1)();       // Allowed. Note that this is logically equivalant to [rand, ctx $f, $t::C]
}
```

# User Experience - Semantics

## Basic Function Declarations

As this feature is fully opt-in, the lack of a context list results in implicity having the `defaults` context.

I.E. the following to definitions are functionally identical:

```
function default_context1(): void {...}
function default_context2()[defaults]: void {...}
```

## How Contextful Functions Interact

Contexts represent a set of capabilities. A list of contexts represent the set union of their capabilities. In order to invoke a function, one must have access to all capabilities required by the callee. However, the caller may have more capabilities than is required by the callee, in which case simply not all capabilities are "passed" to the callee.

This is perhaps more easily seen via example:

```
function pure_fun()[]: void { /* has {} capability set */}
function rand_int()[rand]: void {/* has {Rand} capability set */}

function rand_fun()[rand]: void {
  pure_fun(); // fine: {} ⊆ {Rand}
  rand_int(); // fine: {Rand} ⊆ {Rand}
}

function unannotated_fun(): void {
  rand_fun(); // fine: {Rand} ⊆ {IO, Rand, Throws<mixed>} aka the default set
}

function pure_fun2()[]: void {
  rand_fun(); // error: {Rand} ⊈ {}
}
```

Above, `rand_fun` is logically safe, as its ability to do nondeterministic computation shouldn’t prevent it from invoking functions without that ability. However, note that `pure_fun2` is unsafe, as it does not have that capability, and therefore must refrain from invoking those that require it.

Now consider the following:

```
function pp_coinflip()[io, rand]: void {
  pretty(conflip());
}

function coinflip()[rand]: bool {
  return rand_int() == 0;
}

function pretty(bool $b)[io]: void {
  print($b ? "heads" : "tails");
}

function pure_fun()[]: void {
  pp_coinflip(); // whoops
}

```

The invocation of `pp_coinflip` from `pure_fun` is obviously unsafe, as invoking `pure_fun` could, in fact, actually result in impure actions. Therefore, functions with the pure context only invoke other functions with the pure context. Note, however, that `pp_coinflip` is fine to invoke `coinflip` and `pretty`.

## Subtyping & Hierarchies

Semantically, capabilities work as if they were required parameters
to functions, and are thus contravariant.  This means that, for example,
a closure that requires a `[rand]` or `[]` (pure) context may be passed
where the expected type is a function that requires `[rand, io]`.
(The converse is disallowed because that would mean giving an
additional capability for randomness out of thin air.)

The errors then fall out by normal subtyping rules by internally
treating permissions as (implicit) arguments of a function/method.

```
class Parent {
  public function maybeRand()[rand]: void {...} // {Rand}
  public function maybePure(): void {...} // {Throws<mixed>, IO, Rand}
}

class Mid extends Parent {
  public function maybeRand()[rand]: void {...} // {Rand} -> fine {Rand} ⊆ {Rand}
  public function maybePure()[io]: void {...} // {IO} -> fine {IO} ⊆ {Throws<mixed>, IO, Rand}
}

class Child extends Mid {
  public function maybeRand()[]: void {...} // {} -> fine {} ⊆ {Rand}
  public function maybePure()[]: void {...} // {} -> fine {} ⊆ {IO}
}
```

In the above, the contexts on the methods in `Parent` and `Child` are required for `Mid` to typecheck successfully. Note also that `maybePure` in `Parent` need not be the pure context, and that `maybeRand` in `Child` need not be `rand`.

### Capability subtyping

In reality, there may also exist a subtyping relationship between
capabilities; suppose that a new capability `FileInput` is defined.
Since reading from a file does *not* preclude one from reading
a special file such as `/dev/random` on a UNIX-like system,
the semantic model should conservatively assume that a function
with capability `FileInput` must also have the `Rand` capability.
Therefore, `FileInput` must be a subtype (subcapability) of `Rand`.

This has an important consequence that falls out by design:
whenever some capability `B` that is subtype of capability `A`
is available (in scope), any function (or operation) that
requires `A` can be called (or performed, respectively).

## Interaction with closures

By default, closures require the same capabilities as the context in which they are created. Explicitly annotating the closure can be used to opt-out of this implicit behaviour. This is most useful when requiring the capabilities of the outer scope result in unnecessary restrictions, such as if the closure is returned rather than being invoked within the enclosing scope.

```
function foo()[io]: void { // scope has {IO}
  $callable1 = () ==> {...}; // requires {IO} - By far the most common usage
  $callable2 = ()[] ==> {...}; // does not require {IO}, requires {}
  $uncallable1 = ()[rand] ==> {...}; // does not require {IO}, requires {Rand}
  $uncallable2 = ()[defaults] ==> {...}; // does not require {IO}, requires the default set
}
```

Note that in the previous example, `$uncallable1` cannot be called as `foo` cannot provide the required `Rand` capability. `$callable2` is invocable because it requires strictly fewer capabilities than `foo` can provide.

## Higher-order Functions With Dependent Contexts

Higher-order functions are typically used for generalization purposes, with common examples including standard `map` and `filter` functions. For these functions, a common pattern is to generalize over the inputs and/or outputs of their function-typed arguments. It is imperative that the addition of contexts does not remove this generalizability while maintaining simplicity of their definitions.

Consider the following higher-order function declaration and calling functions. In order to maintain generality, safety, and backwards compatibility, the end result needs to be that `good_caller` and `nocontext_caller` should typecheck while `bad_caller` should not. We solve this problem via the use of dependent contexts, defined above.

```
function callee(
  (function()[_]: void) $f,
)[rand, ctx $f]: void {... $f(); ...}

function good_caller()[io, rand]: void {
  // pass pure closure
  callee(()[] ==> {...}); // callee is {Rand}

  // pass {IO} closure
  callee(()[io] ==> echo "output"); // callee is {Rand, IO}
  // pass {IO, Rand} closure
  callee(() ==> echo "output"); // callee is {Rand, IO}
  callee(() ==> {...}); // callee is {Rand, IO}
}

function bad_caller()[]: void {
  // pass {} closure but tries to do IO
  callee(()[] ==> echo "output"); // // C is {} -> callee is {Rand}
  // pass {} closure
  callee(() ==> {...}); // C is {} -> callee is {Rand}
}

function nocontext_caller(): void {
  // this closure requires the default context
  callee(() ==> {...}); // callee is {Rand, Throws<mixed>, IO}
}
```

Note that, logically, this suggests and requires that all other statements within `callee` require only the `Rand` capability, as the actual `C` passed cannot be depended upon to be any specific capability (and can in fact be the empty set of capabilities).

A potentially more compelling example is the `Vec\map` function in the Hack Standard Library.

```
function map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1)[_]: Tv2) $value_func,
)[ctx $value_func]: vec<Tv2> { ... }
```

## Partially Contextful Hierarchies aka Context Constants

### Background

It is not uncommon to want your API to accept an object implementing an interface and then invoke a method appearing on that interface. That is fine in the common case, wherein the context of that function within the hierarchy is invariant. However, it is possible for a hierarchy to exist for which it is difficult or impossible to guarantee that all implementations of a method have the same context. There are a good number of these situations within the Facebook codebase, but the easiest example is the `Traversable` hierarchy.

The following is an oversimplification of the hierarchy and methodology for how `Traversable`s work in Hack. Do not consider this an actual reference to their inner workings. Consider a `Traversable` interface that defines a `next` function which gives you the next item and an `isDone` function that tells you if there are more elements.

```
interface Traversable<T> {
  public function next()[???]: T; // what do we put here?
  public function isDone()[]: bool; // always pure
}
```

The most common children of `Traversable` are the builtin `Containers`, with children like `vec` and `Map`. However, non-builtin objects are allowed to extend `Traversable` as well, creating arbitrarily traversable objects.

```
interfact Container<T> implements Traversable<T> {
  public function next()[]: T; // {}
}

final class CreateTenNumbers implements Traversable<int> {
  private int $nums = 0;
  private bool $done = false;
  public function isDone()[]: bool { return $this->done; } // {}
  public function next()[rand]: int { // {Rand}
    invariant(!$this->done, 'off the end');
    if ($this->nums++ === 10) { $this->done = true; }
    return rand_int();
  }
}
```

Now consider the following function:

```
function sum(Traversable<int> $nums)[]: int { // Has {}!!!
  $sum = 0;
  while(!$nums->isDone()) {
    // if $nums is CreateTenNumbers, this is unsafe!
    $sum += $nums->next(); // hmmmmm
  }
  return $sum;
}
```

This code should not typecheck! The `sum` function has no capabilities, but what are the capabily requirements of the call to `next`?

### Solution

The solution to this problem is the capability constants described above; the idea simply being that the interface has an abstract capability list, usable on methods of the interface, and concretized by children. In our case, we would use such a capability constant to describe the `next` function:

```
interface Traversable<T> {
  abstract const ctx C;
  public function next()[this::C]: T;
  public function isDone()[]: T;
}

interface Container<T> implements Traversable<T> {
  const ctx C = [];
}

final class CreateTenNumbers implements Traversable<int> {
  ...
  const ctx C = [rand];
  public function next()[rand]: int { ... }
  ...
}

function sum(Traversable<int> $nums)[$nums::C]: int { ... }
```

In fact, the `Vec\map` function above would likely actually look something like this:

```
function map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1)[_]: Tv2) $value_func,
)[$traversable::C, ctx value_func]: vec<Tv2> { ... }
```

As with normal type constants, one cannot override a concrete capability constant.

### Context Constant Defaults

As with normal type constants, we don’t want to force all children to specify the constant. Thus we expose abstract context constants with defaults.

```
abstract const ctx C = [defaults];
```

Here, the first non-abstract class that doesn’t define a concrete context for `C` gets `[defaults]` synthesized for it.

## Local operations

Examples of coeffect-enforced operations include printing,
static property or superglobal access, as well as throwing exceptions.
For output such as from invocation of the `echo` built-in, the `IO`
capability must be locally available in the function/method body.

```
function io_good()[io]: void {
  echo "good";  // ok: {IO} ⊆ {IO}
}
function io_bad()[]: void {
  echo "bad";  // error: {} ⊈ {IO}
}
```

## Parameterized Contexts / Capabilities

Thus far, we’ve covered examples of relatively simple contexts, effectively representing a binary option. However, contexts must also be usable for situations requiring more than that binary choice. Consider, now in more detail, the `throw` context defined above. Rather than describing that a function can throw, this would describe `which` classes of exceptions a function can throw. In that scenario, the context would require a parameter representing the exception class: `throws<-T as Exception>`.

One might note, however, that it would be reasonable for a function to throw multiple kinds of exceptions, often without a unified hierarchy. While it is possible to use an additional interface to unify those exceptions, that would quickly result in a combinatorial explosion. Instead, the result would look like this:

```
function throws_foo_exception()[throws<FooException>]: void { // {Throws<FooException>}
  throw new FooException();
}

function throws_bar_exception()[throws<BarException>]: void { // {Throws<BarException>}
  throw new BarException();
}

function throws_foo_or_bar_exception(bool $cond)[
  throws<FooException>, throws<BarException> // {Throws<FooException>, Throws<BarException>}
]: void {
  if ($cond) {
    throws_foo_exception();
  } else {
    throws_bar_exception();
  }
}
```

The above would indicate that `throws_foo_or_bar_exception` may throw any of the listed exception classes.

This also applies to to the introduction of additional instances of a parameterized context due to dependent contexts:

```
function throws(
  (function()[_]: void) $f,
)[throws<FooException>, ctx $f]: void {...}
```

In the above, if `ctx $f` is `throws<BarException>` then `throws($f)` would be `{Throws<FooException>, Throws<BarException>}`.

# IDE experience:

The IDE experience for this feature will be similar to that of standard types. Autocomplete, syntax highlighting, etc will all be available.

The error messages will be customizable for each context such that the errors make it clear

1. What’s missing in order to typecheck function calls successfully
2. What illegal operation is taking place within the body of a contextful function.

For example, attempting to `echo` within a function having the pure context would simply indicate that doing IO is not permissable due to that context.

# Implementation details:

## Typechecker

The proposed system is modeled as implicit parameters in the
typechecker. These parameters do not exist at runtime, and the
locals that carry them are not denotable by users (`$#` prefix).
The parameters are automatically looked up and checked during type
inference of call expressions via the `$#capability` local.
Coeffect-enforced local operations (such as I/O and throwing)
look up and check if the `$#local_capability` has the
appropriate type.  Multiple capabilities are encoded using
an intersection type, i.e., a capability set `{C1, C2, C3}`
would be represented as `(C1 & C2 & C3)`, assuming an
oversimplification (for now) that there exists a one-to-one mapping
between contexts and same-named types.  In either case,
the *available* capability needs to be a subtype of the *required*
one in order for type-checking to succeed (note that a set is a
subtype of any of its supersets).  This way, we reuse existing
typing infrastructure, and get subtyping for free.

```
namespace HH\Capabilities {
  interface A {}
  interface B extends A {}
  interface A2 {}
}

function f(/* A $#capability */)[A]: void {
  f(/* $#capability */); // ok
  g(/* $#capability */); // bad, A </: B
}   //                  available^     ^required

function g(/* B $#capability */)[B]: void {
  g(/* $#capability */); // ok
  f(/* $#capability */); // ok (B <: A)
}   //                 available^    ^required

function h(/* (A & A2) $#capability */)[A, A2]: void {
  g(/* $#capability */); // bad (A & A2) </: B)
  f(/* $#capability */); // ok  (A & A2) <:  A)
}   //                  available^^^^^^      ^required
```

So, a function with a `mixed` coeffect has *none* of the capabilities we define, and can be called by any function. By comparison, a function with a nothing coeffect has every capability, but can only be called by other functions whose coeffect is `nothing`. In practice, we won’t use `nothing`, but rather an intersection type that covers a *default* set of capabilities, namely `defaults`.

### Mapping of contexts to capabilities

The intended place to define new contexts and capabilities is an
`.hhi` file under `hphp/hack/src/hhi/coeffect`.  In the same directory,
there is also a GraphViz visualization that concisely describes the
relationship between contexts and capabilities system in a way
that may be more understandable to an average Hack developer.

The kind of coeffects coincides with the kind of types.  The syntactic
piece `[C1, ..., Cn]` is interpreted during naming as follows. Each
annotation `Ck` must either be:

* a *fully* namespace-qualified type using the preferred CamelCase naming,
  e.g., `\HH\Capabilities\Rand`;

* or an *unqualified* type representing a *context* using a snake case name,
  e.g., `rand`, which is an alias to one or more of the above
  (multiple types are intersected on the right-hand side)

The former would be sufficient and constitutes a sound (co)effect system;
however, we encourage uniformly using the latter instead because
it provides a general mechanism that facilitates top-down migrations
(more details on that below).

In case a context requires and provides multiple capabilities, there
are two choices:

* declare a (sealed interface) supertype of the desired capabilities; or
* declare an alias and use intersection types (preferred).

Declaring an alias to an intersection of capabilities (or other
aliases) is strongly preferred as it allow the requirements to be
constructible in multiple ways; e.g., if a context `composite_context`
maps to capabilities (and contexts) `Cap1` (`context1`) and `Cap2
`(`context2`), then the following code would still work fine as expected,

```
function callee()[composite_context]: void {}
function caller()[context1, context2]: void {
  callee(); // ok (composite_context = context1 & context2)
}
```

unlike the former hierarchy-based approach
(`CompositeCap extends CompositeCap1, CompositeCap2`)
that would fail with the following message:

```
This call is not allowed because its coeffects are incompatible with the context (Typing[4390])
  ... context of this function body provides the capability set
    {Capability1, Capability2}
  ... But the function being called requires the capability
    CompositeCapability
```

### Enforcing local operations

To enforce local operations (such as throwing exceptions),
the typechecker keeps track of a fake local variable
`$#local_capability`. At the beginning of `f`'s body, it has type:

`\HH\Contexts\context1 & ... & \HH\Contexts\contextN`

where `context1` through `contextN` are the listed on `f`:

```
function f(/*params*/)[context1, ..., contextN]: ReturnType
```

Since each context `contextI` maps to some underlying set of
capabilities, the type of `$#local_capability` simplifies to:

`Cap1 & ... & CapK`

This kind of coeffect(s) is tracked orthogonally to the one used to
enforce calling conventions and they are always sound; no context
can *unsafely* conjure a capability to perform a local operation.

### Enforcing calling conventions

To establish calling conventions in the typechecker,
user-denoted contexts are mapped to two sets of capabilities:

- *C*: intersection of types define by `\HH\Contexts\contextI`
  (exactly as described in the previous subsection)
- *Cunsafe*: intersection of types defined by
  `\HH\Contexts\Unsafe\contextI`
  (analogous to the above modulo the namespace resolution)

Then the `$#capability` local is typed as `(C & Cunsafe)`;
intuitively, this means that the set of available capabilities
for performing a function/method call is the set union of
the safe and unsafe counterparts of each context, where `mixed`
corresponds to an empty set (i.e., it is no-op).  Notably,
this means that usage of a context is *always* sound if the
underlying type `\HH\Contexts\Unsafe\context = mixed`.
On the contrary, `\HH\Contexts\Unsafe\context = (C1 & C2)`,
for example, would mean that the usage of the context is unsound
as it *unsafely* conjures capabilities `C1` and `C2`,
thereby potentially allowing calls into contexts that require
capabilities `C1` and `C2` that would otherwise not be available.

The errors then fall out by normal subtyping rules by internally
treating `$#capability` as an implicit function/method argument
that has type: `C & Cunsafe`.

### A comprehensive example

```
// defined in an .hhi
namespace \HH\Contexts {
  type io = IO;
  namespace Unsafe { type safe_context = mixed; } // == {}

  type unsafe_context = (Rand & Throws<Exception>);
  namespace Unsafe { type unsafe_context = IO; }
}

// user code (WWW)
function do_io()[io]: void {
  // $#local_capability has type: IO
  // $#capability has type: (mixed & IO) = IO
  echo(/* $#local_capability */) "good";  // ok (IO <: IO)
}
function cannot_make_local_io_but_can_call_into_io_ctx()[
  unsafe_context
]: void {
  // $#local_capability has type: Rand & Throws<Exception>
  // $#capability has type: IO & (Rand & Throws<Exception>)
  if (coinflip(/* $#capability */)) {      // <: Rand (ok)
    echo(/* $#local_capability */) "bad";  // </: IO
  } else {
    do_io(/* capability); // ok
  } //       ^ (Rand & Throws<Exception>) & IO <: IO
}
```

### Capturing of capabilities/contexts

When a context list is omitted from a lambda, the type-checker
does not need redefine the two coeffects mentioned above
(for calling and for performing local operations); instead
it exploits capturing of `$#capability` and `$#local_capability`
from the *enclosing scope* (since they are local variables).
This enables memory and CPU savings during type-checking.

Observe that there is no semantic difference between inheriting
capabilities on lambda vs capturing (some of) them from the
enclosing function/method, e.g.:
```
function pp_coinflip()[io, rand]: void {
  $coinflip = () ==> { // capture `rand` */
    // VS.    ()[io, rand] ==>
    return rand_int() == 0;
  };
  ...
}
```

Such capturing is disallowed when context list is present
by merely overwriting the locals with the types resolved
during the context resolution into capabilities.  Partial
capturing (i.e., capturing some capabilities but explicitly
requiring others through the context list) is anyway
discouraged for reasons explained in the [vision document](https://www.internalfb.com/intern/diffusion/FBS/browsefile/master/fbcode/hphp/hack/facebook/vision_docs/tracking_effects_and_coeffects.md):

```
function with_rand_and_io()[rand, io]: void {
  // doesn't make much sense (has benefits of neither)
  $bad = ()[rand] ==> { /* capture `io` */ ... };
}
```

## HHVM

### Runtime Semantics of Co-effects

This section will discuss the user observable behavior of co-effects in the runtime. Note that this section does not assume the code type checks via the Hack typechecker as Hack is not a fully static and sound language as well as the typechecker can be bypassed via HH_FIXME and other means. That being said, this section does assume that the code is syntactically correct. For syntactically incorrect programs, the parser will produce an error that will invalidate the entire file in which the syntactical incorrectness is present.

Co-effects will be enforced natively as part of the calling convention; however, the enforcement will be opt-in by the author of the co-effect. This means that each co-effect will always be enforced or never enforced. We will not allow Hack developers to opt out of enforcement; however, we will selectively provide local and shallow enforcement for migratory purposes as well as a global mode where we raise warnings instead of exception. Using the local, shallow or warning level enforcement will not allow you to use features that are enabled by the enforcement but it will be a stepping stone for full enforcement (See migration section for more information).

We will separate co-effects into two buckets from the runtime’s perspective: 1) Erased co-effects, 2) Enforced co-effects.

The erased co-effects represents co-effects where the enforcement is not needed for correctness in runtime nor gives any benefit to the runtime. Hence these co-effects do not need to be enforced in the runtime. These will be dropped from runtime after their syntactical correctness is ensured

The enforced co-effects represents co-effects where the enforcement is required in order to establish correctness and power potential features such as reactive cache and implicit contexts respectively. From here onwards, we are going to assume the co-effects discussed will all be enforced co-effects.

Runtime will have a native knowledge of each co-effect. We will not allow co-effects to be defined in user-land code. All possible list of co-effects will be defined in the runtime via the RuntimeOptions. In addition to their definition, the enforcement level(which includes whether we raise a warning, throw an exception or nothing upon enforcement failures) will also be configurable via the RuntimeOptions to enable safe migration. An important aspect to note here is that certain co-effects that need deep support from runtime such as `io` and `pure` will need to be implemented in the runtime (i.e. runtime is aware of them without the RuntimeOptions definition) and RuntimeOption definition of it only denotes the enforcement level of these co-effects.

These restrictions are in place due to performance and correctness requirements of the implementation. Correctness requirement is presence and absence of some of the enforced co-effects enable usage of various features discussed above and in order to grant access to these features, runtime needs to natively know these co-effects. Performance requirement is that the runtime will need to implement some sort of validation and the number of co-effects present will heavily influence the cost of this check. Specific runtime strategies for this check is discussed in implementation details section.

The runtime will enforce the co-effects as a part of the calling convention. The exact order of these checks is still subject to change as most of these checks do not depend on each other for correctness but for those that do, the runtime will create the best topological sorting of the checks. The list of checks that happen as a part of calling convention are the following in their current order:
1) Function name resolution
2) Visibility checks
3) Inout arity and parity checks
4) Forbidden dynamic-call checks
5) Reified generic arity/parity checks
6) Function parameter arity checks
7) Co-effect enforcement
8) Parameter type enforcement

This means that co-effect enforcement errors will be triggered prior to parameter type errors and other similar errors that trigger the error handler. Co-effect enforcement error will trigger a `BadMethodCallException` that can be caught in the userland to recover.

If preferable, one may think of the runtime tracking of coeffects as an implicit parameter containing a bitset of active capabilities that is passed to and from all function invocations. At the call boundary, a special check is made to ensure that the parameter as requested is sufficiently provided by the caller. Following that check, however, the passed in implicit parameter is not simply forwarded, but logically replaced by one exactly matching the requirements of the called function. This is similar to "dropping" the unrequired coeffects from the passed in implicit parameter, such that they may not be used inside the callee even if provided by the caller.

### Why enforce in HHVM at all?

One may ask why it is necessary to enforce these guarentees at runtime. The language as a whole is generally unsound, and we're working to progress that forward. Why not just add this to the list of pieces that will get iteratively fixed with the rest of the language? Further, what terrible thing actually happens if the rules aren't enforced dynamically?

The answer to the first question is relatively simple. The planned system for a sound dynamic type requires that types implementing it make some guarantees about their properties, methods, etc, such that writing into one via a dynamic type won't result in unsoundness. If all functions have unenforced capabilities, then they would all necessarily preempt a type from implementing dynamic.

There are multiple planned contexts for whom the goal is to make strong guarentees about chain of trust. Without dynamic enforcement, chain of trust is broken and those guarentees aren't successful. This would be tantamount to a constant recurring SEV.

### Rules received by HHVM and how they get translated from the syntax

As a part of bytecode emission, HHVM will receive a list of rules for how to compute the enforced capability set for each function declaration (including closure declarations). The final capability set for an invocation of that function is the set union of the results of computing the rules.

The rules are as follows:

1. **STATIC\<set of zero or more capabilities>**: The set of statically known capabilities.
2. **FUN_ARG\<arg num>**: Argument <arg num> is a function-type argument. Read the list of capabilities for the passed in function. Read the empty set if the value is null. Fatal if the argument is not null, a function pointer, a closure, or a PHP-style reference to a function.
3. **CC_ARG\<arg num, context constant name>**: Argument <arg num> is an object that defines a concrete context constant named <context constant name>. Read the contents of that context constant. Read empty set if the value is null. Fatal if the context constant doesn’t exist, is abstract, or if the argument is not null or an object. Arrays are special-cased to have any abstract context constants defined in their class hierarchy.
4. **CC_THIS\<context constant name>**: `this` defines a concrete context constant named <context constant name>. Read the contents of that context constant. Fatal if the context constant doesn’t exist or is abstract.

Given a list of coeffects [*concrete<sub>1</sub>*, ... *concrete<sub>n</sub>*, *this::C<sub>1</sub>*, ... *this::C<sub>n</sub>*, *ctx $arg<sub>a</sub>*, ... *ctx $arg<sub>z</sub>*, *$arg<sub>a</sub>::C<sub>1</sub>*, ... *$arg<sub>z</sub>::C<sub>n</sub>*]

* *concrete<sub>k</sub>* -> STATIC<{concrete<sub>k</sub>}>
* *ctx $arg<sub>k</sub>* -> if *$arg<sub>k</sub>* is the name of argument *i* -> FUN_ARG<*i*>. If no arguments are named *$arg<sub>k</sub>*, raise an error.
* *$arg<sub>k</sub>::C<sub>l</sub>* -> if *$arg<sub>k</sub>* is the name of argument *i* -> CC_ARG<*i*, *C<sub>l</sub>*>. If no arguments are named *$arg<sub>k</sub>*, raise an error.
* *this::C<sub>k</sub>* -> CC_THIS<*C<sub>k</sub>*>

Following generation, the STATIC rules are set-unioned into a single rule, and rules are otherwise deduplicated syntactically.

Some examples are as follows along with the emitted list of rules for that declaration.

```
function f(mixed $x)[io, rand] {}
Rules: [STATIC<{io, rand}>]

function f((function()[_]: void) $x)[io, ctx $x] {}
Rules: [STATIC<{io}>, FUN_ARG<0>]

function f(
  (function()[_]: void) $x1,
  (function()[_]: void) $x2,
)[io, ctx $x1, ctx $x2] {}
Rules: [STATIC<{io}>, FUN_ARG<0>, FUN_ARG<1>]

function f(Something $x)[io, $x::C] {}
Rules: [STATIC<{io}>, CC_ARG<0, C>]

function f(Something $x1, Something $x2)[$x1::C, $x2::C] {}
Rules: [STATIC<{}>, CC_ARG<0, C>, CC_ARG<1, C>]

public function f()[this::C, rand] {}
Rules: [STATIC<{rand}>, CC_THIS<C>]

public function f(Something $x1)[$x1::C, this::C] {}
Rules: [STATIC<{rand}>, CC_ARG<0, C>, CC_THIS<C>]

public function f(
  Something $x1,
  (function()[_]: void) $x2,
)[$x1::C, ctx $x2, this::C, IO] {}
Rules: [STATIC<{io}>, CC_ARG<0, C>, FUN_ARG<1>, CC_THIS<C>]
```

### What HHVM does with the given rules

HHVM receives a list of rules from the bytecode emitter and processes these rules to create a set of capabilities that will be used to enforce the capabilities of the function call. Note that HHVM will have a list of capabilities but not a mapping between capabilities and the generics that Hack sees which means that the runtime will not not have access to the original forms of the capabilities, generics and context types. This means that the capabilities cannot be reified, used in is/as expressions or accessed via reflection.

HHVM will convert each rule into a set of capabilities and union the sets generated by each rule to finalize the set of capabilities for the function. When HHVM is unable to convert a rule into a set of capabilities, a `BadCapabilityException` will be raised.
Next, we will discuss how each rule gets converted to a set of capabilities and we will follow it with how HHVM compares the rules for a function call.

For all the following rules, runtime will enforce that <arg num> field is within the boundaries of the function and that <context constant name> exists and is not abstract. One more aspect to note here is that only certain type of defaulted arguments may be used for capability rules: null and compile time known static Hack Arrays. This is a limitation of the runtime due to performance reasons as runtime needs to enforce capabilities prior to executing the default value initializers.

1. **STATIC\<set of zero or more capabilities>**

This rule is the most straight forward of the rules. Runtime will iterate over the capabilities and add them to the result set. If any of the capabilities listed on this list is not natively known to the runtime, runtime will raise an exception.

2. **FUN_ARG\<arg num>**

Runtime will access the closure object, or the function pointer after loading it, associated with the function in argument slot <arg num> and extract its set of capabilities and return it. Runtime will store the capabilities on the closure object.

3. **CC_ARG\<arg num, context constant name>**

Runtime will fetch the argument slot <arg num> and access the late static bound class of this object. On the runtime class object, runtime will return the set of capabilities named <context constant name>.

4. **CC_THIS\<context constant name>**

Runtime will access the runtime class of the current function and return the set of capabilities named <context constant name>. If the current function is not a non-static method, `BadCapabilityException will` be raised.

Once set of capabilities are extracted from each rule, the runtime will union the capabilities and generate a final list of capabilities for the function. Note that this list of capabilities contains unique capabilities that are each known to the runtime. At this point, no capability is polymorphic or based off of some other context. We will call this set of capabilities the **ambient set of capabilities** of the function. As a result of this, polymorphic co-effects cannot be be referenced inside the body of the function, as they have already been resolved at that point.
Due to runtime limitations, specifically, runtime not being able to distinguish between each polymorphic co-effect since runtime operates over the entire set of co-effects, polymorphic co-effects cannot be be referenced inside the body of the function.

Now we will discuss how the runtime enforces a function call based on these set of capabilities. Note that from here on the runtime is operating over the ambient set of capabilities, we will no more need to discuss polymorphism or any other co-effect capabilities.

When a function `f` calls a function `g`, the runtime will fetch the ambient set of capabilities for each function. In order to enforce the correctness of capabilities, the caller environment must be able to satisfy all the capabilities of the callee. More concretely, in order for the function call from `f` to `g` to happen, the ambient set of capabilities of `f` must be a superset of that of `g`. If this is true, the function call will happen with the enforcement guarantee. Otherwise, the runtime will raise an exception, raise a warning or silently ignore based on the enforcement level specified by the compiler configuration.

Due to runtime limitations, specifically, runtime not being able to distinguish between each polymorphic co-effect since runtime operates over the entire set of co-effects, polymorphic co-effects cannot be be referenced inside the body of the function.

### End-to-end Example

In this section, we will walk through a real life example end-to-end to illustrate each component. Note that the following example has two function calls to `map_with_logging`, one good and one bad. We will discuss how and why the good one passes the enforcement and similarly how and why the bad one fails.

```
function map_with_logging<Tv1, Tv2,>(
  Traversable<Tv1> $traversable,
  (function(Tv1)[_]: Tv2) $value_func,
)[io, $traversable::C, ctx $value_func]: vec<Tv2> { ... }

function caller_good(vec<int> $v)[rand, io]: void {
  map_with_logging($v, ($i)[rand] ==> generate_rand_int());
}

function caller_bad(vec<int> $v)[rand]: void {
  map_with_logging($v, ($i)[rand] ==> generate_rand_int());
}
```

First, the bytecode generator will generate the following rules for each function.

```
Rules for map_with_logging: [Static<{io}>, CC_ARG<0, C>, FUN_ARG<1>]
Rules for caller_good:      [Static<{rand, io}>]
Rules for caller_bad:       [Static<{rand}>]
```

Secondly, the runtime will extract the ambient set of capabilities of each function.

```
caller_good: [Static<{rand, io}>] => {rand, io}
caller_bad:  [Static<{rand}>] => {rand}
```

Now, let’s inspect map_with_logging for each call.

```
map_with_logging called by caller_good: [Static<{io}>, CC_ARG<0, C>, FUN_ARG<1>]
Static<{io}> => {io}
CC_ARG<0, C> => {}
FUN_ARG<1>   => {rand}

Ambient: {io} | {} | {rand} = {rand, io}
```

```
map_with_logging called by caller_bad: [Static<{io}>, CC_ARG<0, C>, FUN_ARG<1>]
Static<{io}> => {io}
CC_ARG<0, C> => {}
FUN_ARG<1>   => {rand}

Ambient: {io} | {} | {rand} = {rand, io}
```

Notice in both examples the ambient set of capabilities of `map_with_logging` resulted in `{rand, io}`. This is due to both functions passing the lambda argument with same capabilities.
Lastly, we will execute the enforcement section.

```
Ambient set of capabilities of caller_good: {rand, io}
Ambient set of capabilities of map_with_logging called by caller_good: {rand, io}

{rand, io} is a superset of {rand, io}. Function call succeeds.

Ambient set of capabilities of caller_good: {rand}
Ambient set of capabilities of map_with_logging called by caller_bad: {rand, io}

{rand} is NOT a superset of {rand, io}. Function call fails.
```

### Eager versus Lazy Enforcement of Conditional Co-effects

The above implementation discusses the eager enforcement of conditional co-effects. Another way to enforce co-effects would be to lazily enforce them. This type of enforcement changes the user observe-able effects of the enforcement since with eager enforcement note that we enforce the lambda on the above example when the lambda is passed as an argument, whereas; with lazy enforcement the enforcement would be done when the lambda is used. This also means that with lazy enforcement more programs would be valid as if the lambda is not used, the enforcement would never be done. Lazy enforcement would allow us to enforce more programs that would otherwise be disallowed with eager enforcement such as passing a vector of co-effectful lambdas. With eager enforcement, since the runtime currently does not enforce inner type of containers, we would disallow such programs from being syntactically correct; however, with lazy enforcement since we’d be enforcing them when the lambda is used, we could allow them.

It is worth re-iterating that lazy enforcement would potentially allow other types of illegal programs to execute:

```
function callee((function(): void)[_] $f, dynamic $d)[ctx $f] {
  // callee inherits only the co-effects from $f (io in this case)
  // $d is a dynamic function pointer to a function that requires the rand co-effect
  // Because of lazy enforcement we simply forward all co-effects from
  // caller() to callee() and work them out later. In this case, we erroneously
  // pick up the "rand" co-effect which would allow the invocation of
  // $d to succeed incorrectly.

  $d();
}

function caller()[io, rand] {
  callee(() [io] ==> print(1), random_int<>);
}
```

We have decided the employ the eager enforcement since we believe that it will be more performant for the runtime and less complex since after each function call, we know that everything is enforced and we do not need to keep track of what needs to be enforced.

Since eager enforcement is stricter than lazy, it will be possible to switch to lazy enforcement if needed in the future and we could employ other strategies to get the benefits of lazy enforcement as well by introducing some sort of runtime tagging of containers.

### Trade-offs and Restrictions

Historically, adding enforcement to the runtime (parameter and return type enforcement, upper bound generic enforcement, property type enforcement, etc) has often come at a cost as the associated checks are not always free. Ideally the information we gain from this enforcement improves the efficiency of generated code by allowing the JIT to specialize for the enforced properties. This benefit will in many cases offset the cost of the enforcement by allowing the JIT to specialize in places where without enforcement the search space would be too large. In this case, however, the runtime cannot take advantage of the co-effect enforcement to improve code emission. Later, we will discussed some strategies to mitigate possible runtime cost that of this enforcement. Before that we will discuss trade-offs and requirements in order to enforce co-effects with minimal runtime cost.

1. Emit significantly more code for these functions: Allowing any arbitrary function to have these polymorphic co-effects would lead to code bloat and instruction cache problems. Our assumption is that polymorphic co-effects will only be common for library functions, and since they are extremely hot and mostly inlined, this would be a good tradeoff.
2. Have the runtime aware of each possible co-effect: This means that each co-effect must be defined in the runtime.
3. Have a hard limit on how many co-effects there can be: A bounded set of co-effects may be efficiently enforced in O(1) time, while an unbounded set of co-effects will require O(n) checks on each call. The chosen bound will be influenced by call ABI and implementation specific constraints within the runtime.
4. Build tracking of more types than we currently do: Currently HHVM does not do any sort of tracking of types for lambdas and type constants. In order to efficiently enforce the above requirements we would need to build such tracking in HHBBC as well as the JIT so that the aforementioned specialized translations can be done. This is, in general, a good direction for the runtime; however, it is also massive amounts of work.
5. Future co-effects which may influence runtime behavior and have additional correctness requirements: An example to this would be, without going into too much detail, banning memoization of functions that use polymorphic co-effects and co-effect type constants. For co-effects such as cipp, we will be depending on the enforcement to assure correctness for implicit contexts. Since for memoized functions we need to know at JIT time whether to memoize the context or not, and we cannot specialize on this for correctness reasons. Any function that must be memoized along with the context must be annotated with non-polymorphic coeffect list, such as [cipp] and have the __Memoize attribute. Runtime will not need to determine dynamically whether to capture the context during memoization.
6. We will disallow partially forwarding polymorphic co-effects: This is discussed with an example above.

# Motivating usecases

## Purity

Pure code is not able to do very much, but most standard library functions fit neatly into the model. The most important aspect of an example for this is that both pure and impure code can safely and cleanly utilize the pure framework code.

For hack, as above, the pure context is simply one with the empty context list.

```
function map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1)[_]: Tv2) $value_func,
)[$traversable::C, ctx value_func]: vec<Tv2> { ... }

function double_it(vec<int> $v)[]: vec<int> { // {}
  return map($v, $i ==> $i*2); // C -> {}, Ce -> {}, overall -> {}
}

// impure code -> not an error
function double_and_print_it(vec<int> $v)[io]: vec<int> {
  return map($v, $i ==> {
    $doubled = $i*2;
    echo "$doubled\n";
    return $doubled;
  });
}

// ERROR (both with and without [io] on lambda)
function whoops(vec<int> $v)[]: vec<int> {
  return map($v, ($i)[io] ==> {
    $doubled = $i*2;
    echo "$doubled\n";
    return $doubled;
  });
}
```

## CIPP

For CIPP, the most important capability is having access to the runtime Purpose Policy. This is a capability that needs to be added to the context by setting the implicit. Most likely, this will be done using a magic builtin function:

```
function with_implicit<TIn, TOut>(
  TIn $implicit,
  (function()[implicit<TIn>]: TOut) $f
): TOut {
  ..
}
```

Here, the `implicit<T>` capability gives code the ability to access a value of type `T` at runtime. The `cipp<T>` capability is an alias signaling that code should have access to an implicit `T` and also must be analyzed for data leaks. The API for entering into a CIPP context might look like this:

```
function eval<Tcipp, T>(
  Tcipp $purpose_policy,
  (function()[cipp<Tcipp>]: T) $f,
): Wrapped<Tcipp, T> {
  ...
}
```

Developers would then do something approximating this:

```
$wrapped_result = Cipp\eval(Policy::MY_POLICY, () ==> {
  // Do My Policy things
  ...
});
```

# Design rationale and alternatives:

The [vision document on tracking (co)effects](https://www.internalfb.com/intern/diffusion/FBS/browsefile/master/fbcode/hphp/hack/facebook/vision_docs/tracking_effects_and_coeffects.md)
motivates our decision to use a *capability*-based **coeffect** system
as opposed to alternatives documented there.

## Why this needs to be a language feature

The major question to consider in this section is whether the feature is truly necessary or whether it is possible to emulate this within the language presently. It is generally possible to emulate the cross-function interaction via exposing the hierarchies of permissions in user-space, and then mimicking the behavior with explicit parameters. However, the user experience of that would be absolutely horrendous, especially consider the planned usage of this in such things as the Hack Standard Library. Further, implementing the dependent contexts behavior in user-space, if even possible, would be very complicated and likely extremely confusing.

However, even if one were determined to implement the cross-function behavior of contexts in user-space, there is no way to replicate the special typing rules and effects on function bodies that this proposal provides. Consider the pure context - logically, this requires banning the usage of the `echo` statement within the body of that function. This is only implementable via a lint rule, which will never be 100% effective, resulting in any given context being transitively untrustable.

## Drawbacks

The current design involves checking an extra argument at every function invocation in the codebase. Because the types in question are big intersections, this has the potential to considerably slow down overall typechecking.

Initially, the default context will have 2-3 capabilities (permission for external mutable references, global state, and to call reactive/pure code). Reactive code would have 1 capability and pure would have 0. Multiple capabilitys are currently represented via an intersection type in the typechecker, so we need only a single subtyping check for each function/method call, albeit with a large constant factor as subtyping type intersections (and unions) could be expensive.

Options to mitigate this (within the typechecker, not shown to users):

1. Special-case the most common cases to avoid full intersection type subtyping/simplification, e.g. if both caller and callee is unannotated, both are fully pure/CIPP, etc. This is probably simplest optimization to try and introduces a fast path and makes the slow path a tiny bit slower.
2. Ad-hoc replace intersections of common combinations of capabilites with a single interface type, so that the check is cheap. This wouldn't be too conservative (incomplete but sound) if we can prove that 2 or more capabilities in the intersection of interest cannot be introduced independently (i.e., they always appear in tandem). See subsection "Encoding multiple capabilities" in the Vision doc

# Prior art:

See this [vision document](https://www.internalfb.com/intern/diffusion/FBS/browsefile/master/fbcode/hphp/hack/facebook/vision_docs/tracking_effects_and_coeffects.md), section Prior art, for existing systems, which also deal with the challenge of minimizing the burden on users via a “lightweight” syntax.

## Effects in Haskell

Haskell is one of the few programming languages that already has an effect system. Although effects in Haskell are monadic rather than capability-based, we can still learn about from how they are used. Haskell functions are pure by default and Haskell has many of the same effects that we have discussed in this document including failure (Either), implicit arguments (Reader), mutation (State), nondeterminism (Random), and IO. Most monads are single purpose, but the IO monad can do nearly everything. IO in Haskell is similar to the default set of capabilities in Hack.

Although Haskell has monad transformers which allows users to combine effects, the common usage pattern is to use a single Monad if you need a single effect, or IO if you need many effects. It is fair to assume that the same will be true in Hack; the best practice should be to use default rather than specifying a long list of individual effects.

# Unresolved questions:

## Tracking Dynamic calls

A major issue for this proposal, as well as hack generally, is the existence of dynamic invocations. This is worse for contexts, as a dynamic invocation down the stack breaks the transitive guarantees up the stack. A pure function might actually result in IO due to some dynamic call far down the stack. This is not a problem for contexts whose calling conventions HHVM will enforce, but that will almost certainly not be universally the case.

Somewhat ironically, the solution for this is an additional context: `can_dyn_call`. `can_dyn_call` gives the capability to invoke functions on receivers that are not known to hack to be static function types. Common examples include `HH\dynamic_fun` and invoking off of `mixed` via a `HH_FIXME`.

```
function call_dynamicly_bad(string $funname, mixed $fun): void {
  // HH_FIXME[4009]
  $fun('foobar'); // this would be unfixmeable
  HH\dynamic_fun($funname)('foobar');
}

function call_dynamicly_safe(string $funname, mixed $fun)[can_dyn_call]: void {
  // HH_FIXME[4009] - still needs to be fixmed
  $fun('foobar'); // but no additional unfixmeable error
  HH\dynamic_fun($funname)('foobar');
}
```

This would extend to other types of dynamic invocations as well, such as invoking off a dynamic type, TAny, etc.

Note that this context gives a capability to functions, therefor requiring it to be present all the way up the stack from the entry point to the call that invokes the dynamic function.

Unfortunately, even the above is not sufficient due to the following case:

```
function callit(
  (function()[_]: void) $f,
)[ctx $f]: void { $f(); }

function breakit(mixed $fun): void {
  /* HH_FIXME[4110] */
  callit($fun); // whoops
  /* HH_FIXME[4110] */
  callit(HH\dynamic_fun($fun)); // still whoops
}
```

Therefore, we would also require an additional annotation: whether a function type *may* contain a value that would trigger a dynamic invocation error if used directly.

In the above case, something like the following would be required:

`function callit(<<__MaybeDynFun>> (function(): void) $f): void { $f(); }`

Combing the above two features allows us to generally trust the transitive promises of unenforced contexts.

There is technically one further issue, but we do not believe it is a blocking one:

```
function callit((function(): void) $f): void { $f(); }

function breakit(mixed $var_containing_callit, mixed $fun)[can_dyn_call]: void {
  /* HH_FIXME[4009] HH_FIXME[4110] */
  $var_containing_callit($fun); // whoops
}
```

Note that in the above case, this is only possible because `breakit` is marked as `can_dyn_call`, so even though `callit` isn’t marked as such, we’re not accidentally introducing a capability not provided by the call stack. This is similar to the question of coercibility for like types.

However, due to the above, we would need to ban passing mismatching function-type args unless the caller’s arg is marked as `<<__MaybeDynFun>>`.

This work would necessarily be a prerequisite for adding sound capabilities whose calling conventions aren’t guaranteed by the runtime.

Further questions: Does the ability to do `$x->foo = $my_fun` where `$x` is unknown break this because `$foo` could be not marked as `__MaybeDynFun?` Do we need to hardban this? What does this mean for generics, reified generics, and higher kinded types?

Even with all of the above, it might not generally be possible to guarantee safety in all cases, meaning that we can’t soundly roll out unenforced contexts.

# Future possibilities:

One can imagine any number of applications for this feature. This is quite a powerful and flexible feature that can be used to represent a number of things, depending on our wishes. Some ideas are given below.

## Use Capabilities to Model Async/Await

Rather than the current version:

```
async function gen_do_things(): Awaitable<void> {
  await gen_other_things();
}
```

We could model the permission to use await as a capability:

```
function gen_do_things()[async]: Awaitable<void> {
  await gen_other_things();
}
```

Besides uniform treatment (resulting in a simpler overall system), this would allow for abstracting over synchronicity (i.e., `async` vs normal). E.g., the following snippet would type-check:

```
function do_later((function()[_]: void) $work)[ctx $work]: void { ... }

function with_async_io()[async, io]: void {
  do_later(() ==> print("quick"));  // captures `io` capability
  do_later(() ==> await slow_fun());  // captures `async` cap.
  do_later(()[async] ==> await slow_fun()) // explicitly has `async` cap
}
```

One could further use dependent contexts and have fine-grained callbacks:

```
function wrap_callback(
  (function()[_]: void) $f
):(function()[ctx $f, io]: void) {
    return () ==> {
      $ret = $f();
      print("done");
      return $ret;
    };
}
function demo_callback(): void {
  wrap_callback(() ==> 42);
  wrap_callback(()[async] ==> await slow_fun());
}
```

## Modules

Modules are a proposed Hack feature that would enforce the boundaries between Prod, Intern, and Test code. The idea is that the typechecker would prevent developers from calling from Prod into Intern or Test because that code is not available on production machines and would otherwise fail at runtime. This feature can be easily implemented using capabilities. Prod code simply would not have the permission required to call into intern or test code.

```
// file_in_prod.php
module Prod; // implicitly gives [prod] context

function calls_into_intern()/*[prod]*/: void {
  callee_in_intern(); // ERROR
}
function callee_in_prod()/*[prod]*/: void { ... }

// file_in_intern.php
module Intern; // implicitly gives [intern] context

function calls_into_prod()/*[intern]*/: void {
  callee_in_prod(); // OK
}
function callee_in_intern()/*[intern]*/: void { ... }
```

## Ability to log to Stdout/Stderr

An unfortunately common mistake is one is which a developer adds a `slog`
statement within their code for the purpose of debugging but
then forgets to remove it before committing. We could make the capability
to log to stderr/stdout represented via the context system, such that most
code utilizing it would have a hack error, rendering it uncommitable.
A debugging or development environment could conjure this capability
to facilitate quick development cycle (see the previous subsection).

## Levels of Dynamic Coercibility

In the forthcoming like-types project, there is the concept of coercion for dynamic values. In the base version, there will be some ability for a dynamic value to be passed where another is expect. Much of this is still up in the air, but one exactly is passing a dynamic value into an enforced typehint.

There has been some discussion of having a way to turn off that implicit coercion. We could present this as a capability represented via capabilities, regardless of the desired default.

## Flow-sensitive capabilities

For `throws` context to be fully practical, we would need to refine the
local and calling capabilities depending on the language constructs.
Specifically, a `try`-block would introduce the `Throws` capability
regardless of whether the function provides it, and the keyword
`throw` would require that capability or error otherwise, e.g.:

```
function try_once<T> {
  (function()[throws<Foo>]: T) $f
)[rand]: ?T { $t = null;
  try { // adds Throws<mixed> as a local & calling capability
    if (coinflip()) {
      throw "ok";    // {Rand, Throws<mixed>} ⊆ {Throws<mixed>}
    } else {
      $t = $f(); // ok: {Rand, Throws<mixed>} ⊆ {Throws<mixed>}
    }
  } catch(...) { // Throws<Foo> is no longer available here
    throw "bad"; // error: {Rand} ⊈ {Throws<mixed>}
  }
  return $t;
}
```

It is questionable if the run-time would every support throwing, since
capability violations result in exceptions (chicken and egg problem).
However, the typechecker could track the capabilities as follows:

```
function try_once<T> {
  (function()[throws<Foo>]: T) $f
)[rand]: ?T {
  $t = null;
  //           $#local_capability = Rand
  try {     // $#local_capability = Rand & Throws<mixed>
    if (...) {
      throw(/* $#local_capability */) "ok";
            // $#local_capability = Rand & Throws<mixed>
            //                          <: Throws<Foo>
    }
    // $#capability = $#capability & $#local_capability
    //             == Rand & Throws<mixed>
    $t = $f(/* $#capability */); // ok: Rand & Throws<mixed>
                                 //         <: Throws<mixed>
                                 //         <: Throws<Foo>
  } catch (...) { // $#local_capability = Rand
      throw(/* $#local_capability */) "bad"; // error
            // $#local_capability = Rand </: Throws<Foo>
  }
  return $t;
}
```
