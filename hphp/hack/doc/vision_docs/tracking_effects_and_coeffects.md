Title: **Tracking effects and coeffects** via capabilities

Start date: July 7, 2020

Status: DRAFT

# Summary

The purpose of this proposal is twofold:

* unify and systematically model reasoning about *effects,* including
  reactivity (all flavors), purity, determinism, global state; and
* build a sound foundation for contextual properties, formally known
  as *coeffects,* such as (implicit) viewer contexts or environments

The proposed *capability-based* approach has the unique benefit that
it can model both effects and coeffects. The main idea is to enhance
the typechecker with a limited form of *implicit parameters* (a simple
coeffect type system) and desugar into and pass capabilities that act
as permissions for effects or resource/context access. It has very
little impact on the Hack syntax and almost no impact to the users.

Although capabilities have been employed in many security systems
since the last century, it wasn’t until 2015-16 that this idea was
welcomed in the programming languages (PL) community to solve a larger
problem of (co)effects. Around that time, two independent PL research
labs have not only formalized their interestingly similar
capability-based type systems; they also implemented it as a *small*
language extension/plug-in of Scala and shown that the approach scales
to large & complex code base of 100K+ lines (Scala Standard
Library). See section “Prior art” for details.

To be clear, this proposal is not about exposing the feature of
implicit parameters to the Hack users (yet); it is about building a
*typechecker primitive* that enables other complex features (both
existing and upcoming) to be modeled elegantly and soundly in the
typechecker, thus enabling Hack to be more expressive. For
example, the (conditional) reactivity/purity would not each
require special context-propagation logic and subtyping and
conformance (caller-callee) rules — everything would simply just “fall
out” from the typing rules with only a minor addition — a capability
type is read from an additional context carried along with regular
typing context. A similar desired behavior can be achieved for contexts
such as normal/experimental by mapping them into implicitly passed
capabilities that statically prevent unsafe calls from non-experimental
code into experimental code (for stability and future compatibility).

# Motivation

As outlined in the Summary, this would provide a reusable mechanism to
neatly and soundly express existing features:

* reactivity & recently introduced purity,
* *conditional* context propagation / treatment of effects in general

and in addition it would enable to easily start tracking a new kind of
effect, such as IO, non-determinism, etc.  For that, a new capability
*domain* is to be introduced, with the appropriate subtyping
relationship. E.g., to track input/output that can be further refined
into disk-based and network-based I/O, the subtyping of capabilities
would be:

```
CanDisk <: CanStdIO // capability for accessing local disk
CanNet <: CanStdIO  // capability for access network/Internet
CanIO <: (CanNet | CanDisk)  // ~bottom type for effect domain IO
```

Now the basic idea is to require that an appropriate capability (or a
subtype) is available in scope, either via being:

* required by a so-called *stoic* function/method; or
* captured from a *contextful* (potentially effectful/impure) function.

The prominent way to propagate these capabilities is to treat them as
*implicit* parameters, as section Prior Art explains.

***Note***:
The syntax `@{Capability1, ...}` below is just a **desugaring** and to
illustrate how typechecker would see a function signature; it is
intentionally chosen to resemble the standard “Γ **@ C**” notation for
coeffects (as opposed to “T ! E” of effects). Comment
`/*{Capability}*/` denotes capabilities that are filled in by the
typechecker, which is akin to passing implicit arguments as it has no
implications on the user code.

```
// 2 stoic functions that require different capabilities
function log2disk(string $message)@{CanDisk}: void { ... }
function publish_paste(string $text)@{CanNet}: void { ... }

function log_and_publish_as_paste(string $msg)@{CanIO}
  log2disk($message)/*{CanIO}*/;      // ok: CanIO <: CanDisk
  publish_paste($message)/*{CanIO}*/; // ok: CanIO <: CanNet
}
```

## Effect polymorphism

Effect polymorphism is the property that enables writing a single
definition for a function/method whose effect can vary depending on
the arguments passed. Contexts and associated capabilities can be seen
as (co)effects. E.g.:

```
function handle_messages(
   vec<string> $msgs,
   // denoted by: => in [FLiu’16-17]
   //             @local in [OOPSLA'16]
   <<__Contextful>> (function(string): void) $log
): void { ... }

function pure_context(): void {
  handle_messages(vec["a", "b"],
    // no capability flows in
    (string $msg) ==> {});
}
function disk_context()@{CanDisk}: void {
  handle_messages(vec["a", "b"],
    // lambda captures the capability from outer scope
    (string $msg) ==> log2disk($msg)/*{CanDisk}*/ );
}
```

The new attribute `<<__Contextful>>` will be described shortly; its
idea is to allow passing *both*:

* functions that *not* capture/require context (e.g., pure ones), and
* contextful functions that capture some context,

which enables effect polymorphism. This *by design* solves the
*conditional* context propagation, which is just a
specific instance of effect polymorphism (with reactivity being
considered as effect). For convenience, we could also support
desugaring of `log2disk` into the explicitly written lambda in the
last function above.

***Note***: `<<__Contextful>>` is can be internally desugared from the
existing syntax for most use cases such as reactivity, as it will be
explained later.

Compared to desugaring into generics, distinguishing between the two
kinds of functions at the type level provides benefits similar to the ones
explained in [Rytz'14]§2.2 and [ECOOP'12]. First, it enables a
clean mapping from a lightweight user syntax (considered in a separate HIP)
and type-checking errors does not involve mismatches of inferred types.
Second and more important, provides a means to accept a function that
makes arbitrary effects in *any* domain without impact on the user syntax.
For example, adding a new effect domain (e.g., what exception is thrown,
whether it is deterministic, what type of IO, etc.) would require:
- inferring the new capability in existing code for every call site,
  essentially regressing type-checking whenever a new effect domain is added;
- new syntax to be placed whenver we want to account for the new domain.

The classic example is Java, where in order to consider a new
domain of effects such as Exceptions, you need to add a new version of
function parameterized by additional generic `E` as follows:
```
<E> void handle_messages(
  Vector<String> msgs,
  FunctionThatThrows<String, Void, E> log
) { ... }
// NOTE: N+1 definitions needed for N domains of effects
void handle_messages(
   Vector<String> msgs,
   Function<String, Void> log
) { ... }
```
Therefore, the generics-based mapping does not scale well; this is further
explained later on and in materials listed under section Prior art.

Fortunately, only a handful of functions, namely higher-order functions,
will surface this distinction, so the impact of the semantics and syntax
is very limited compared to the clear benefits it provides for the users,
as well as the soundness and performance of the typechecker.

### Current ad-hoc *conditional* effects

Without the proposed way of supporting effect polymorphism, the above
snippet would need to be expressed like this:

```
<<__CanIO, __AtMostIOAsArgs>>
function handle_messages(
   vec<string> $msgs,
   <<__CanDisk, __AtMostIOAsFunc>> (function(string): void) $log
): void { ... }
```

Besides annotation burden, this clearly does not scale in terms of
implementation effort because each use case of capability *Cap*
necessitates special handling of a brand new set of attributes such
as: *AtMost{Cap}AsArgs, AtMost{Cap}AsFunc, Only{Cap}IfImpl*, etc. If
encoded with the old (current) system, the same challenge applies for
every new effect domain or contextual information propagation.

## Subtyping

Current use cases that would be superseded by capabilities, notably
reactivity/purity, specially type-check function calls and methods, by
checking if a function (or method) can soundly call another (or
override, respectively). This challenge is often phrased as “calling
conventions”.

With the newly proposed system, one could also instead rely solely on
the subtyping of capabilities, considering implicit parameters are in
*covariant* position. The desired behavior would simply “fall out”
from normal typing rules, provided that capabilities are treated as
the last/first argument in the typechecker:

```
function handle_messages(
   vec<string> $msgs,
   (function(string)@{CanIO}: void) $log
): void { ... }

function pure_context(): void {
  handle_messages(vec["a", "b"],
    // absence of capability is same as top type
    // note: {} is equivalent to {CanNothing}
    (string $msg)/*@{}*/ ==> {});
}
function disk_context()@{CanDisk}: void {
  handle_messages(vec["a", "b"],
    // ok: function(string){CanDisk}:void <: function(string){CanIO}
    //     because CanIO <: CanDisk
    log2disk);
}
```

However, such *monomorphic* handling is *not* fine-grained — it
doesn’t tell much about what precise effects the `handle_messages`
function can make besides “any IO effect”, which is equivalent bottom
type for this particular capability lattice of IO effects. Introducing
generics for capabilities would not only complicate inference without
solving the problem, but it would also introduce a large cognitive
overhead & burden for the Hack users, as section Prior art (and
associated literature) confirm:

```
function handle_messages<C super CanIO>(vec<string> $msgs,
   (function(string)@{C}: void) $log
): void { ... }
```

# User experience

## Syntax

This proposal by itself doesn’t require any changes to the parser. The
`<<__Contextful>>` attribute would just be specially interpreted by
the typechecker. The rest are just typechecker changes that impact
the developers of Hack.

To decouple this proposal from others, this proposal uses a
self-contained *desugared* syntax as explained above. To repeat, the
`@{Capability}` part is *not* the user syntax and neither is the
`__Contexful` attribute — it is just the way how typechecker sees the
signatures of functions and methods.

All syntax used in this proposal is for explanatory purposes and a
future HIPs on contexts and reactivity/purity will explain the *new*
syntax and how it maps to the implicit capabilities in this proposal.

## Semantics

The Hack users will need to be aware that capturing a capability from
a lambda makes it more restrictive. In the [FLiu’16-17] such
*contextful* functions are referred to as
non-stoic/free/impure. Conversely, stoic functions cannot capture a
capability but *can* require some. In [OOPSLA’16], contextful
functions are treated as 2nd-class, similar to procedures in Algol and
Pascal but still less restrictive.

This proposal calls for distinguishing stoic/monomorphic functions vs
contextful/polymorphic functions at the *type* *level*. E.g.:

```
function higherOrder(
  (function(): int) $monoStoicAndPure,
  (function()@{CanThrow}: int) $monoStoic,
  <<__Contextful>> (function(): int) $poly,
  <<__Contextful>> (function()@{CanNonDet}: int) $poly2,
): void
```

While the syntax can be massaged so that this distinction is opaque at
a syntactic level, users do need to be aware of the semantics. The
first 2 argument functions are stoic, meaning that they cannot close
over (capture) a capability. The last 2 argument functions are not;
they are effect-polymorphic in a sense that they represent a
computation created with *arbitrary context*, thus capturing any
number of capabilities.  Here is an example of passing each kind of
argument function:

```
function throwingIOstuff()@{CanThrow,CanIO}: void {
  higherOrder(
    /*$monoStoicAndPure=*/() ==> 42,

    /*$monoStoic=*/ ()@{CanThrow} ==>
      throw/*{CanThrow}*/ new Exception(),

    /*$poly=*/() ==>
      fopen("/non/existing/file")/*{CanThrow}*/
        ->readInt()/*{CanIO}*/,

    /*$poly2=*/()@{CanNonDet} ==>
      randomUpTo(2)/*{CanNonDet}*/ +
      Console\readInt()/*{CanIO}*/,
  );
)
```

In fact, implicitly passed capabilities are a restricted form of *coeffects.*

### Implicit arguments as coeffects

To understand the need for imposing restrictions on
capability-capturing lambdas, it helps to illustrate how implicit
parameters behave. They can be bound at either *declaration* site and
*call* site, e.g.:

```
function nondeterministic_context()@{NonDet}: void {
  // capability captured by the lambda that has no context
  $decl_site_bound = () ==> randomUpTo(1)/*{NonDet}*/
}
function deterministic_context()@{}: void {
  // capability required at each call site
  $call_site_bound = ()@{NonDet} ==> randomUpTo(2)/*{NonDet}*/
  ...
}
```

The former may be fine for certain (future) use cases, perhaps
environments, but requires careful consideration. If we allowed this
for effect capabilities, this could lose track of effects if the
capability escapes:

```
function wrap_random_as_deterministic(
)@{NonDet}: (function(): int) {
  return () ==> randomUpTo(1)/*{NonDet}*/
}

void nondeterministic_context(...)@{NonDet}: void {
  $some_class->property = wrap_random_as_deterministic()
}
void deterministic_context(...): void {
  ($some_class->property)() // leak
}
```

but the soundness ultimately depends on whether:

* capabilities can ever be introduced “out of thin air” akin to
  *unsafe* blocks in Rust or *FIXMEs* in Hack; and
* lifetime of a capability can be extended beyond declaring *scope.*

### Implicit capabilities as effects

Therefore, capability-based effect systems implemented in a host
language where capabilities are first-class, i.e., regularly typed
variable, need to reintroduce some restrictions. Two such systems are
[FLiu’16-17] and [OOPSLA’16] as the section Prior arts explains.

The former is a simpler but more restrictive option, quoting key bits
from Section 3:

> we need to change the definition of the function pure to **exclude
> free function types from the pure environment**. This restriction is
> important, because there’s no way to know what side effects there
> might be inside free functions. **If stoic functions have access to
> free functions, we’ll loose the ability to track the effects of
> stoic functions in the type system.**

Porting the former system to Hack would mean treating all functions as
non-stoic/impure initially, allowing them to capture capabilities.

The latter system is implemented in Scala, with capability being
represented as implicit parameters, therefore it reimposes a
*2nd-class* value discipline (the challenge is also known as the
[funarg problem](https://en.wikipedia.org/wiki/Funarg_problem)) on
such capabilities as well as capability-capturing functions. The
[[OOPSLA’16]](https://dl.acm.org/doi/pdf/10.1145/2983990.2984009)
paper describes the rationale on pages 1-2:

> Second-class values in the sense of ALGOL have the benefit of
> following a strict stack discipline (“downward funargs”), i.e., they
> cannot escape their defining scope. This makes them **cheaper to
> implement** ...  Since **first-class object**s may escape their
> defining scope, they **cannot be used to represent static
> capabilities** or access tokens – a task that second-class values
> are ideally suited to because they have bounded lifetimes and they
> have to “show up in person”.

Porting that to Hack would mean:

* treating capability-capturing lambdas (e.g., `$decl_site_bound`) as
  2nd-class, preventing them from being:
    * stored in a property; or
    * returned (to be reconsidered, see end of previous subsection).

(Some of these restrictions are reconsidered under Unresolved questions.)

In either porting direction, we would need to:

* gradually change some functions to track effects via capabilities
  desugared from the old attribute syntax or the new context syntax;
* work around restrictions reimposed for soundness by
  over-approximating effects, e.g., implicitly desugaring some
  attributes or functions to have `CanEverything` capability.

As the following snippet demonstrates, this latter approach is more
expressive in a sense that we can still pass around such contextful
(impure) function.

```
function nondeterministic_context()@{NonDet nondet}: void {
  // note: $get_int is no longer 1st-class
  $get_int = () ==> randomIntUpTo(5){nondet};

  // it can still be passed around safely nonetheless
  eagerly_call($get_int);
}

function eagely_call(<<__Contextful>> (function(): int) $f) {
  $f(); ...
}
```

However, the treatment of capabilities as non-1st-class
(`<<_Contextful>>`) correctly disallows untracked effects.

```
new Store($get_int) // ERROR: expected stoic but got contextful
...
class Store {
  function __construct(public (function():int) $callback) { ... }

  // note: omitting the attribute wouldn't allow passing a stoic function
  function store(<<__Contextful>> (function(): int) $f) {
    this->$callback = $f; // ERROR: expected stoic got contextful fun.
  }
}
```

Note that we do not need to expose the 2nd-class terminology to the
users, we can just explain how contextful functions are different from
stoic functions, which is needed for adopting either of the two
approaches!

# Implementation details

For most use cases, we wouldn’t require support in the runtime. As an
example, reactivity and purity of functions is already tracked with a
special bit in HHVM, so they don’t need to rely on having runtime
values for implicit capabilities/contexts.  Consequently, capabilities
can be tracked purely *statically*, and the run-time does not need to
know about the existence of implicit parameters — unless we decide to
unify it with a pending HHVM proposal on implicit contexts.

The feature can be implemented in the typechecker in several mostly
*non-overlapping* phases:

1. implement distinction between regular (stoic) lambdas and
    effect-polymorphic/contextful ones (the distinction is also needed
    for alternative #1)
2. implement desugaring of contextful functions (those that can
   capture capabilities) passed to *higher-order* functions as
   2nd-class or impure/non-stoic:
    1. either via an existing attribute syntax
    2. or a new *contexts* syntax, to be proposed in a separate HIP;
3. modify the typechecker environment to also carry capabilities
   (multiple, or a single one using an intersection type)
4. modify the typing rule for function application to check if
   the required capability is available in the (implicit) context;

Once phase 2 is finished, we should add these annotations to WWW,
which would allow for smoke-testing
phases 3.-4. (as well as 1.).  As a part of phase 2, we may need to
implement a subset of functionality of 2nd-class values, but this is a
relatively easy task:

* built in 2 days during a Hackathon
  (even with user syntax that is not needed here);
* also implemented and tested in another language, Scala in *only*
 [**400 lines**](https://github.com/losvald/scala/blob/3070ee4931f0429a80a517a0c167028ed3e5865d/src/compiler/scala/tools/nsc/typechecker/EscLocal.scala#L65)
 in typechecker &
 [10 lines](https://github.com/losvald/scala/blob/9495ebca55f712ae3459fe5eb86ffab2477fde8a/src/library/scala/Esc.scala)
 in standard library (see section Prior art for details)

For prototyping & unit-testing, we actually also modified the
parser to support parsing Concrete Syntax Tree directly into
capabilities, but that should obviously be gated so that WWW cannot
use it / be aware of this exploit.

### Encoding multiple capabilities

Multiple capabilities that express permissions to make effect in
multiple domains (e.g., determinism, exceptions, reactivity, etc.),
are encoded via an intersection type. E.g., `NonDet & Throwing`
represents having *both* the capability for non-deterministic
execution as well as the one for throwing exceptions,
respectively. Since `NonDet & Throwing <: NonDet`, it satisfies the
requirement of / privilege for non-deterministic
execution. Intuitively, intersection gives the caller more privilege
(not less) and soundly approximates multiple capabilities.

Unannotated functions in Hack should implicitly have
some default set of capabilities, at the very least
the capability to call into non-deterministic code
(including other unannotated functions or methods)
and permit untracked mutation (such as global state),
neither of which is allowed in reactive/pure code.

Considering this simplified scenario, we encode this set of
capabilities and give it a name using an intersection type.

```
type CanDefaults = (CanNonDet & CanMutateUntracked);
```

The benefit of using intersection type as opposed to
interface inheritance is apparent considering
a hypothetical call site that passes each of
the comprising capabilities *separately*, e.g.:

```
// implicitly @{CanNonDet,CanMutateUntracked}=CanDefaults
function randomBool(): bool { ... }

class CachedNonDetFactory {
  private static ?seed = null;

  function withNonDetBool<T>()@{CanMutateUntracked}(
    (function (
      <<__Contextful>> function()@{CanNonDet}: bool
    ): T) $continuation
  ) {
    return $continuation(()@{CanNonDet} ==> {
      // captures the outer capability: required for mutatting $seed
      self::$seed ??=/*{CanMutateUntracked}*/ currentUnixTime();

      // passes the explicit as well as the captured capability
      randomBool()/*{(CanNonDet & CanMutateUntracked)}*/
    });
  }
}
```

Conversely, if we relied on *nominal subtyping* and defined

```
interface CanDefaults extends CanNonDet, CanMutateUntracked {}
```

then the call to `randomBool` would result in a typing error because
`CanNonDet & CanMutateUntracked` would *not* be a subtype of
`CanDefaults` anymore.

### Default capabilities as an unsound migration mechanism

The capability passing as described so far is essentially the
same as implicit parameter passing in several other languages.

```
function randomUpTo(int $n)@{CanNonDet}: int
```
would be modeled in Scala as follows:
```
def randomUpTo(n: Int)(implicit $_: CanNonDet): Int
```

An interesting question arises: what if we make some capabilities
optional; i.e., provide a default for the corresponding implicit
parameter?

This is surely unsound for effect tracking as the capability is no
longer required at the call site; in fact, it is a means of obtaining
the corresponding privilege "out of thin air".

In a language with *default* implicit parameters such as Scala,
this would be written as:

```
def randomBoolUnsafe()(implicit $_: CanNonDet = new CanNonDet {}): Int
```

In Hack, we express this using by enhancing our prototypical syntax as
follows:

```
// requires: {} (no capability, i.e., can always be called)
// provides: {CanNonDet}
function randomBoolUnsafe()@{ +CanNonDet}: void { ... }
```

where the part following `+` in the braced list (`{}`) after `@`
denotes the *unsafe* (i.e., provided but not required) capability.

## Feature interactions

This feature would supersede the special-cased (ad-hoc) logic for
type-checking reactive and pure functions.

### Dependent types

The expressibility would be greater if Hack adopted path-dependent
types, see Unresolved questions below.

### Reification & HHVM implicit contexts

Ideally, this should influence the design of HHVM’s proposed but put
“on hold” feature of implicit contexts because it would be nice if we
could one day unify these two via reification:

* non-reified capabilities → only static enforcement in the typechecker;
* reified capabilities → also run-time inspection/retrieval of the
  corresponding context.

See the section Future possibilities for more details.

# Design rationale and alternatives

Implicit parameters offer expressiveness that can be hardly surpassed
by any other feature. Therefore, it’s better to compare alternatives
for problems solvable via implicit parameters, most notably effects.

### Alternative #0: special tags + ad-hoc typechecking logic

This is mostly similar the how the current typechecker reasons about
reactivity.  There is an extra step in typing functions and their call
sites, which examines the tags and validates the rules. However, this
means that subtyping, for example, needs to be special-cased for each
of these reactivity-like tags. Second, many attributes behave
specially and have to be carefully handled at several stages in
parsing & type-checking. Third, this is complicated by fact that we
support what is known as *conditional* reactivity, i.e., a function is
reactive only if its argument function is reactive or a subclasses are
known to be reactive. This clearly doesn’t scale in terms of
implementation and complexity (due to special cases) as soon as one
introduces new form(s) of “context” — essentially new (co)effects:

* purity — cannot call `Rx\IS_ENABLED`, but otherwise stricter than `Rx`;
* IO (e.g., from data source / or to sinks);
* non-determinism and parallelism;
* async, as well as sync (immediately awaited async call);
* contexts that restrict the flow of (sensitive) data;
* exceptions?

There is a general consensus to move away from
this verbose and overly restrictive syntax.  For the alternatives
below, the new syntax for reactivity and purity — or *contexts*
in general — would be *desugared* into more *reusable and
well-known primitives*. This would solve the major challenge of the
current approach — each “domain” of (conditional) context
needs a special type-checking logic that is not
well-integrated with the rest of the type system, as well as
propagation through different stages (bug-prone and inflexible).

### Alternative #1: type-and-effects systems

This is the oldest and most common way of encoding effects. Some early
paper(s) point out different challenges in sound and not overly
restrictive approximation of runtime behavior (especially using
constraint-based solvers?).  The main disadvantage of this approach is
that it is *quite invasive* — nearly every typing rule would need to
be modified — and effects carried in the typechecker. The latter may
lead to significant performance overhead, and the former is also less
than ideal in terms of implementation cost & maintenance effort, as
well as overall complexity, of the typechecker code.

The current proposal requires far *fewer* modification to typing rules;
*only* function call needs to read from the (implicit) capability context,
and *only* function declarations add to it. Conversely, type-and-effect
systems require a type in *every* position to be associated with an effect
(as it can potentially be a function that makes an effect).
The trade-off is somewhat similar to that of virtual dispatch via:
- vtable (context per class) -> additional capability context
  is looked up per each function call;
- [fat pointer](https://en.wikipedia.org/wiki/Dynamic_dispatch#Fat_pointer)
  (every pointer is *doubled* in size) -> encoding of a type
  is bloated by being *paired* with an effect.

Since virtual dispatch in languages with a virtual machine and JIT
has negligible performance overhead (unlike statically compiled ones),
the performance impact on the typechecking is expected to be minimal.

### Alternative #2: monad-based encoding

In [POPL’18], Martin Odersky et al have found that monadic encoding of
“freer” monad suffered
[5x performance hit](https://dl.acm.org/doi/pdf/10.1145/3158130#page=22)
compared to encoding via implicit parameters. This was done in Scala,
where implicit arguments carry slight run-time overhead, meaning that
implementing a subset of implicit parameters functionality that
doesn’t require run-time mechanism in Hack would lead to even bigger
performance wins compared to wrapping types in monads.

### Alternative #3: algebraic effects

Based on my high-level understanding of the relatively few papers that
talk about it:

* `+` provide a modular way for customizing effect handling
* `-` not rigorously verified via large case studies for usability & performance;
* `-` requires special treatment in [IR](https://dl.acm.org/doi/pdf/10.1145/3341643)/VM to efficiently do [CPS translation](https://dl.acm.org/doi/pdf/10.1145/1291151.1291179)
    (not so practical without invading into HHVM internals)

Even if we invested a lot into building the support for *efficient*
translation to Continuation Passing Style (CPS), which does not look
trivial at all (see Andrew’s [ICFP’07] paper), we would need to
perform more “testing in the wild” as these approaches do not have
comparable user & performance studies, unlike capability-based
(co)effect systems.  Finally, the most well-known approach, published
in [ICFP’13](https://dl.acm.org/doi/pdf/10.1145/2500365.2500581), is
*not* entirely static and require dependent types; and another
practical candidate
([Effekt @ Scala’17](https://dl.acm.org/doi/pdf/10.1145/3136000.3136007))
requires multi-prompt delimited continuations, which would in turn be
inefficient as delimited continuations correspond to monads (see above
why monads are inefficient).

Fortunately, representing effects using a type lattice is both
flexible and modular. Please refer to above examples and read details
can be found in [ECOOP’12] and [OOPSLA’16] papers in section Prior
Art.

## Impact to Hack users & WWW

This proposal does *not* expose implicit arguments as 1st-class
language feature; so Hack users:

* *cannot* pass arbitrary values/contexts implicitly;
* do *not* need to know that contexts, such as (shallowly/locally)
  reactive or deterministic, would be from a type-checking perspective
  propagated just like implicit arguments.

Nonetheless, it does help to create a mental model behind each use case,
by generally describing that:

* contexts internally map to capabilities/permissions to do stuff;
* they are propagated in a similar fashion as implicit arguments in
  Scala and Haskell.

The latter would actually help Hack developers with some experience in
either programming language.

Impact to the average WWW developer is negligible, but power users
may want to know:

1. the rationale for effect-polymorphic functions, explicable to Hack
   developers using “conditional” effect examples;
2. that the syntax for reactive/pure contexts internally
   desugars to implicit parameters;
3. *optional*: new “capability type” if we need to support advanced
   use variable-context class hierarchies (see section Unresolved
   questions).

(1.) is a small price to pay that would allow us to eliminate
annotation boilerplate present. However, this is something that even
alternative approaches require (see section Prior art). Regardless of
the implementation direction (i.e., whether capability capturing is
restricted or requires 2nd-class treatment), we will need to translate
user-facing annotations on higher-order functions into either
effect-polymorphic/2nd-class or monomorphic functions. As of June
2020, it seems that only about a thousand function signatures involve
higher-order functions that would require this new type of
annotation(s). Verified via bunnylol: `tbgs (function(`

*Only* the Hack developers will care about:

* capability lattice that enforces *calling convention* for each types
  of contexts such as reactive, non-deterministic, etc., (each type
  corresponds to a use case of implicit arguments internally);

# Drawbacks

I think we should support a foundation for capability-tracking
(co)effects, but the question is how many of the subfeatures should be
present:

* for prototyping, exposing the syntax for ad-hoc in Concrete Syntax
  Tree but gating that parser feature;
* exposing capabilities as types in user code (see section Unresolved
  questions);
* 2nd-class treatment of capability-capturing functions vs disallow
  capturing capabilities from stoic functions (the former is similar
  to how functions in Pascal/Algol behave, see [OOPSLA’16]).

# Prior art

Implicit parameters are a concrete instance of a
[coeffect](http://tomasp.net/academic/papers/coeffects/coeffects-icalp.pdf#page=3)
system, and coeffects are more general than effects (mathematically,
they correspond to indexed comonads as opposed to monads, see
[ICALP’13, JENTCS’08]).

They have been widely accepted as an essential feature of Scala; in
fact a study show that >90% repositories on GitHub use them (see
[Conclusion in [POPL’18]](https://dl.acm.org/doi/pdf/10.1145/3158130#page=26)).
Haskell also supports them
([link](https://www.haskell.org/hugs/pages/users_guide/implicit-parameters.html)),
albeit in a somewhat more restricted form.

In [ScalaDays’15], M. Odersky, the designer of Scala (and a top-notch
PL researcher), pioneered the idea of using implicit *capabilities*
for modeling *permissions*. Implicit parameters are already a feature
in Scala since v2, and overhead of using implicits has been evaluated
on a large corpus of real-world code in [POPL’18], shown to perform
**5x faster than monads**.

Leo's [OOPSLA’16] work has won the Distinguished Artifact Award; it
designs, machine-proves and implements a *capability-based (co)effect*
system using second-class values (they give more expressiveness
compared to the similar approach of Fengyun Liu). It also evaluates
the annotation burden — relevant for our code-mod — showing that
**<2% of code changed** in Scala Standard Library has to be changed to
propagate effects (checked exceptions, parallelism).  The second
author of [OOPSLA’16], G. Essertel has machine-proved in Coq:

* [Simply-Typed Lambda Calculus (STLC) with 2nd-class values (1/2)](https://github.com/TiarkRompf/scala-escape/tree/master/coq#coq-proofs-for-stlc-12);
* [System D **with subtyping** (~Scala\{DOT}) and 2nd-class values](https://github.com/TiarkRompf/scala-escape/tree/master/coq#coq-proofs-for-dsub-12).

(System D with subtyping has been proven sound by Tiark Rompf and Nada Amin in
[*another* OOPSLA’16 paper](https://dl.acm.org/doi/pdf/10.1145/3022671.2984008).)

Fengyun Liu has built a similar but somewhat simpler capability-based
effect system in his PhD dissertation [FLiu’16-17]. The two
simplifications compared to [OOPSLA’16] are (the analogy is parenthesized):

* *effect-tracking* (non-stoic) lambdas *cannot capture* capabilities
  (as opposed to being treated 2nd-class);
* capability parameters are *explicit* (less practical for users & code mod);

He also machine-proved in Coq his approach generalized to STLC,
System F and System D on [GitHub](https://github.com/liufengyun/stoic#stoic).

The common practical challenge of effect systems is known as *effect
polymorphism*, initially described in [POPL’88]. Papers
[ECOOP’12, RytzOdersky’12] show how to encode effects as lattice in a
modular way, and more importantly show how to *annotate higher-order*
functions in a lightweight way that supports effect polymorphism. The
importance of providing effect polymorphism without heavyweight
annotations has been reiterated by a recent work on *gradual*
polymorphic effects in [OOPSLA’15]:

> effect polymorphism, which is crucial for handling common
> higher-order abstractions with enough precision

who implemented
[Effscript](https://pleiad.cl/research/software/effscript) as a Scala
plug-in after extending the theory of [ECOOP’12] (also implemented as
a Scala plug-in).

Annotating signatures of higher-order functions is “**common
denominator**” to all of the approaches above that support effect
polymorphism, namely: [OOPSLA’16], [FLiu’16-17], [ECOOP’12], [RytzOdersky’12].
This is also consistent with Hack’s more verbose
solution for conditional reactivity, which is just a verbose version
of effect polymorphism for the reactivity effect (or in coeffect sense,
a capability for accessing reactive backend).

One of the most practical promising alternative to this proposal,
which is along the lines of [OOPSLA’16] and [FLiu’16-17], is the
type-and-effect system presented in [RytzOdersky’12] and [Rytz’14],
implemented in [EffScript’15], with the idea original idea originating
from [ECOOP’12]. However, the ergonomic treatment of “relative
declaration” requires *dependent* function types as [[Rytz’14]
§3.2](https://lrytz.github.io/download/thesis-rytz.pdf#page=54) points
out, and its implementation is quite complexed judging from
[EffScript’15].  Fortunately, *both* the current proposal and their
approach require the **distinction between** *effect-polymorphic* vs
*monomorphic* functions, which confirms that this should be a
milestone on its own.

[ImplicitsRevisited'19] explains design mistakes of Scala 2 implicits
and reveals how they are being fixed in Dotty and soon in Scala 3.
The current proposals avoids a lot of these by not exposing the
feature as a first-class language construct and instead ports only a
*tiny* subset of the Scala's behavior specifically suited for
passing contextual information and enforcing calling conventions
(but *not* for type classes).

## References:

Most related work was implemented as extensions of Scala, but there’s
also Haskell and others (E-lang, EffScript).

* ScalaSpec: [Scala Language Specification](https://www.scala-lang.org/files/archive/spec/2.11/) (M. Odersky et al.)
* **ScalaDays’15**: [**Scala** - where it came from, where it is going](https://www.slideshare.net/Odersky/scala-days-san-francisco-45917092) (M. Odersky), pages 44-46
* **POPL’18**: [Simplicitly](https://dl.acm.org/doi/pdf/10.1145/3158130) (M. Odersky et al.)
* **OOPSLA’16**: [...Affordable 2nd-Class Values for Fun and **(Co-)Effect**](https://dl.acm.org/doi/pdf/10.1145/2983990.2984009) (Leo Osvald et al.)
    * https://www.dropbox.com/s/iag4gho34ewzk57/OOPSLA16-coeffects_code-changes%2Bresults_README.pdf?dl=1
    * https://github.com/losvald/scala/compare/2.11.x...losvald:esc#files_bucket
    * https://github.com/TiarkRompf/scala-escape/tree/master/coq#coq-proofs-for-dsub-12
* **FLiu’16-17**: [A Study of Capability-Based Effect Systems](https://infoscience.epfl.ch/record/219173?ln=en) (Fengyun Liu), PhD dissertation
    * https://github.com/liufengyun/stoic#stoic
    * https://github.com/liufengyun/stoic/tree/master/2017
* **Haskell’16**: [Effect capabilities for Haskell](https://www.sciencedirect.com/science/article/pii/S0167642315004062#br0170) (Figueroa, Tabareau, Tanter)
* Elang‘98: [The E language](http://erights.org/elang/index.html) (M.S. Miller)
* EffScript’15: [EffScript](https://pleiad.cl/research/software/effscript) (authors of [OOPSLA’15] below)
* ImplicitsRevisited'19: [Lambda World 2019 - Implicits Revisited](https://www.youtube.com/watch?v=uPd9kJq-Z8o) (M. Odersky)
* **SID-1**: [Scala Named and Default arguments](https://docs.scala-lang.org/sips/named-and-default-arguments.html) (L. Rytz), subsection Implicit parameters
* **ECOOP’12**: [Lightweight Polymorphic Effects](http://www.lirmm.fr/~ducour/Doc-objets/ECOOP2012/ECOOP/ecoop/258.pdf) (Rytz, Odersky, Haller)
* RytzOdersky’12: [Relative Effect Declarations for Lightweight Effect-Polymorphism](http://infoscience.epfl.ch/record/175546/files/rel-eff_1.pdf) (L. Rytz & M. Odersky)
* Rytz’14: [A **Practical** Effect System for Scala](https://lrytz.github.io/download/thesis-rytz.pdf) (L. Rytz), PhD dissertation
* OOPSLA’15: [Customizable **Gradual** Polymorphic Effects for Scala](https://dl.acm.org/doi/pdf/10.1145/2858965.2814315) (Toro & Tanter)
* **ICALP’13**: [Coeffects: Unified static analysis of context-dependence](http://tomasp.net/academic/papers/coeffects/coeffects-icalp.pdf) (T. Petricek, D. Orchard, A. Mycroft)
* ICFP’14: [Coeffects: A calculus of context-dependent computation](http://tomasp.net/academic/papers/structural/coeffects-icfp.pdf) (T. Petricek, D. Orchard, A. Mycroft)
* OrchardTalk’14: [Coeffects: contextual effects / the dual of effects](https://www.cs.kent.ac.uk/people/staff/dao7/talks/coeffects-dundee2014.pdf) (D. Orchard, T. Petricek, A. Mycroft)
* OrchardPhD’14: [Programming contextual computations](https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-854.pdf) (Dominic Orchard), PhD dissertation
* ICFP’07: [Compiling with Continuations, Continued](https://dl.acm.org/doi/pdf/10.1145/1291151.1291179) (Andrew Kennedy)
* ICFP’19: [Compiling with Continuations, or without? Whatever.](https://dl.acm.org/doi/pdf/10.1145/3341643) (Y. Cong, Leo Osvald, G. Essertel, T. Rompf)
* Scala’17: [Effekt: Extensible Algebraic Effects in Scala](https://dl.acm.org/doi/pdf/10.1145/3136000.3136007) (Brachthäuser & Schuster)
* ICFP’13: [Programming and Reasoning with Algebraic Effects and Dependent Types](https://dl.acm.org/doi/pdf/10.1145/2500365.2500581) (E. Brady)
* DOT’16: [Type Soundness for Dependent Object Types](https://www.cs.purdue.edu/homes/rompf/papers/rompf-oopsla16.pdf) (Tiark Rompf, Nada Amin)
* POPL’88: [Polymorphic Effect Systems](https://dl.acm.org/doi/pdf/10.1145/73560.73564) (Lucassen & Gifford)
* Talpin‘94: [Type and Effect Discipline](https://www.sciencedirect.com/science/article/pii/S0890540184710467) (Talpin & Jouvelot)
* JENTCS’08: [**Comonad**ic notions of computation](https://www.sciencedirect.com/science/article/pii/S1571066108003435) (Uustalu & Vene)

# Unresolved questions

## Encoding capabilities as (dependent) types

This is something that would be needed to cleanly model
partially reactive class hierarchies (see Future possibilities).

```
class SometimesContextful {
  type CanDo
  function do()@{CanDo}: void
}

class NeverRequiresContext extends SometimesContextful {
  type CanDo
  function do()@{}: void { ... }
}

class AlwaysContextful extends SometimesContextful {
  <<__Override>> type CanDo = CanStdIO
  function do()@{CanDo}: void {
    print("stdout")/*{CanStdio}*/;  // ok: CanDo <: CanStdio
  }
}
```

However, dispatching on such partially contextful class hierarchies
requires dependent types:

```
// generic T would be exposed to users, unfortunately
function maybe_contextful_do<T as SometimesContextful>(
  T $maybe_contextful
)@{
  C = $maybe_contextful::CanDo  // path-dependent (OK)
  // C = T::CanDo  // error, no way to prove that type C is matching
}: void {
  maybe_contextful->do()
}
```

It seems that this could be modeled via Pocket universes, which offer
enum-dependent types, albeit that would require the capability
definitions to be expressed via corresponding enum fields for each
class in the hierarchies, which may not be practical at all.

**Q: Considering such uses cases are fairly common in WWW, should we
generalize dependent types and/or pocket universes?**

## Interaction with lazy collections

This is somewhat related to the above, and has been extensively
studied in [OOPSLA’16] (see Prior Art).  The gist of it is shown by
the following snippet:

```
class Iterable<T> {
  type MaybeContextful;  // used as meta-attribute
  function map<T, U>(
    <<MaybeContextful>>
    (function(T): U) $f): Iterable<U>
}

class Vector<T> extends Iterable<T> {
  type MaybeContextful = __Contextful;
  <<__Override>>
  function map<T, U>(
    <<MaybeContextful>>  // = __Contextful
    (function(T): U) $f): Iterable<U>
}

class Iterator<T> extends Iterable<T> {
  type MaybeContextful = Nothing;
  <<__Override>>
  function map<T, U>(
    // = no attribute (accepts pure functions only!)
    (function(T): U) $f): Iterable<U>
}
```

Note that lazy collections such as subtypes of `Iterator` don’t call
argument function `$f` immediately, they could delay effects,
therefore we don’t want to permit capturing capabilities in their
argument functions passed to `map`. E.g.:

```
function would_hide_effect(
  Iterator<int> $iter
)@{CanNonDet}: Iterator<int> {
  return $iter->map(
    // error: expected contextless (stoic) function
    //     but the passed lambda captures a capability
    (int $x) ==> $x + randomUpTo(1)/*{CanNonDet}*/
  );
}
```

**Q: Is this a better way, or we should stick with alternatives by exposing capabilities as generics (see previous subsection)?**

## 2nd-class/restricted treatment of closures

One can argue that we may not actually need to treat
capability-capturing closures as 2nd-class. The supporting argument
could go along these lines:

* a context (requiring a capability) that once exists cannot disappear; e.g.:
    * the (sub)request has access to reactive backend or it doesn’t
    * execution context is either deterministic or non-deterministic
* a function *cannot* introduce new permissions, it can just opt-out
  of certain permissions by not requiring certain capabilities;
* there is no scope-like facility to introduces a capability type;
* capabilities are not exposed as user types;

The last 2 points were the primary motivation for a conservative
treatment of capability-capturing closures as 2nd-class in [OOPSLA’16]
or impure in [FLiu’16-17] , and also makes the system more generic (if
the first 2 points do not hold).

**Q: Is semi 1st-class treatment of capabilities sound and sufficiently general for our *all* our use cases?**

# Future possibilities

Implicit parameters are one of the simplest forms of coeffects; i.e.,
a “flat” type-and-coeffect system as introduced in the
[ICALP’13 paper](http://tomasp.net/academic/papers/coeffects/coeffects-icalp.pdf#page=3). The
prefix co- stands for **co**ntextual properties and it’s also
essentially a dual of effects (see
[slides](https://www.cs.kent.ac.uk/people/staff/dao7/talks/coeffects-dundee2014.pdf#page=8)).

That means that other language features could be expressed using
implicit arguments under the hood, notably:

* reactive and pure **contexts**
* generic **effects** (by limiting the escaping of capabilities
  through declaration site, see above)
* **modules & features** (contextual by definition):
  (non-)experimental and opt-in features

## Remodel reactivity flavors

There is a one-to-many mapping between reactivity (including purity)
attributes and implicit parameters, e.g.:

* `<<__Pure>>` → `@{CanThrow}` or `@{mixed}` if exceptions untracked
* `<<__Rx>>` → `@{CanRx}`
* `<<__RxShallow>>` → `@{CanRxShallow, ...}`

The *calling conventions* are enforced by design if these capabilities
are organized into a type lattice as follows:

```
CanRxShallow <: CanRx <: CanThrow
```

Intuitively, the more specific the capability type, the higher the
privilege; also some examples:

```
function rx_context()@{CanRx}: void {}
function rx_calling_into_others()@{CanRx}: void {
  // ok: CanRxLocal <: CanRxShallow
  rx_context()/*{CanRx}*/

  // error: CanRxShallow not a subtype of CanRx
  rx_shallow_context()/*{CanRx}*/

  pure_context(); // ok: capability either not passed (mixed) or
  pure_context()/*{CanRxLocal}*/ // passed as a subtype of CanThrow
}
```

Treating pure functions as not having implicit arguments is attractive
from a performance standpoint (typechecker doesn’t need to do extra
work for pure functions), but it is too conservative because pure
functions in the reactive sense do allow certain other kind of
non-conflicting effects such as throwing an exception.


## HHVM’s implicit context as reified capability

Ideally, this should somewhat influence the design of HHVM’s proposed
but postponed because it would be nice if we could one day unify these
two via reification. Some contexts are reified by default such as
reactive ones (tracked via a few bits at run-time), but we may want to
start tracking certain kind of effects statically in the typechecker
and avoid breaking changes to the run-time/VM. The way to do it mark
some contexts/capabilities as non-reified (*internally* in the
typechecker, since there is a finite number of them). E.g.:

```
function with_implicit_context()@{
  ContextOrCapabilityForUseCase1,
  reify ContextOrCapabilityForUseCase2,
}: void {
  // context $ctx2 is available at run-time
}
```

The main benefit of such unification include:

* reified capabilities are *always* part of memoization key (since
  they are available at run-time);
* non-reified capabilities are *never* part of the memoization key;

which hopefully neatly solves the dilemma about impact on performance
and/or correctness of memoization, unblocking the path to finalize the
semantics of implicit contexts. E.g.:

```
<<__Memoize>>  // no need for <<__NoContext>>
function contextless(): void

<<__Memoize>>  // NOT part of memo. key as it isn't reified
function throwing()@{CanThrowException}: void
               // exceptions aren't tracked at run-time

<<__Memoize>>  // IS part of memo. key as it is reified
function reactive()@{reify CanRxIsEnabled}: void
  if (RX\IS_ENABLED/*{CanRxIsEnabled}*/) { ... }
  else { ... }
}
```

In the last function, the `RX\IS_ENABLED` would require implicit
capability when type-checking. Moreover, that capability should be
reified as the function can run both within a disabled and enabled
reactivity backend, therefore its result depends is dependent on the
context.

## Modules & features

Further, we could enforce some encapsulated code such as experimental
cannot be called from a normal *module*, by desugaring these
modules into capabilities:

```
// file1
module Normal;  // Normal is a weaker capability than Experimental

function calls_into_experimental()/*@{Normal}*/: void {
  // ERROR: the call requires capability: Experimental
  //                  but got capability: Normal
  // and Experimental is not a supertype of Normal
  // (i.e., the capability cannot be upcast)
  callee_in_experimental()/*{Normal}*/
}

// file2
module Experimental;

function callee_in_experimental()/*@{Experimental}*/
```
with the following subtyping relationship for this domain of (co)effects:
```
Experimental <: Normal
```

Rules for which module can access which other module can be encoded by
carefully organizing more capabilities into a subtyping, which allows
for encapsulation similar to modules/packages in Scala/Java, respectively.

Interestingly, fine-grained use of experimental features can also be
enforced by requiring appropriate capabilities in a similar fashion,
which is comparable to Rust's tracked usage of (unstable) features.
For example, experimental features of union & intersection types
would be modeled as requiring a file-local capability
`CanUnionIntersection`, desugared from the file-level attribute:

```
<<file:__EnableUnstableFeatures('union_intersection_type_hints')>>
```

where the desugaring would simply inject the capability into the
typing environment used when type-checking anything in that file
(but not other files).


## Haskell-like type constraints

From [[OrchardPhD’14] §6.5 (page
135+)](https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-854.pdf#page=135):

> **Type constraints can be seen as a coeffect** describing the
> contextual-requirements of an expression. ... This section goes
> towards unifying coeffects and type constraints, which is used in
> the next chapter to encode a coeffect system based on sets using
> Haskell’s type class constraints.  ...  The coeffect approach
> describes the implicit parameter requirements of a function as
> latent coeffects, rather than constraints on the type in the
> approach of Lewis et al.
