Title: Ownership and Linearity Tracking

Start Date: June 26 2020

Status: Postponed

## Summary

A system for tracking and enforcement of ownership for Objects, specifically
via linearity.

## Feature motivation

There are multiple existing or future features for which ownership tracking is
required for soundness. The major usecase of this is the Purity project, which
will have its own HIP in the coming weeks. However, there are other features,
such as const classes and disposables, for which language-level ownership
tracking is necessary for soundness. We expect there to be additional future
features that can make use of this system.

Additional benefits include:
1. HHVM can produce optimizations based on the guarantees described below when utilizing these features.
2. Provability that an API requesting a unique value is actually given one

## User experience

### The Four States of Ownership

There are four states of ownership:
1. Unowned (The default)
2. Owned
3. Borrowed
4. MaybeOwned (temp name, looking for alternatives).

The main differences between each state are the operations allowed on an object
of that state. Ownership is a property of the linearity of an object. Owned and
Borrowed objects are enforcibly linear, Unowned objects are not, though they may
be natuarally, and MaybeOwned may or may not be forcibly linear.

Described in terms of the operations allowed on them, the states are as follows:
1. Unowned - The default state in hack. May already have aliases
(i.e. be nonlinear). May be freely aliased.
2. Borrowed - Cannot be aliased and is a non-owner of the object. The owner
of the object is a (possibly indirect) caller, so no other code with a reference
to this object can run until this function returns (or further lends the object
to a more deeply nested callee). Because of this restriction, we consider the
object to still be linear in this state. If the caller did not opt in to stricter
tracking aliases may already exist, but the borrowing code will not make new ones.
See the next section for more details
3. Owned - Does not have aliases and cannot be aliased. May be lent to a
(synchronous or immediately awaited async) callee, which preserves the invariant
that at most one runnable frame has a reference. Can transfer ownership and can
release ownership, moving the object into the unowned state.
4. MaybeOwned - May be an object in any of the previous 3 states. This state is
distinct from Borrowed because some features built on ownership tracking (purity
and const classes) need to allow mutations only on linear (i.e. Owned or
Borrowed) values. Functions that only need to read from an object will be more
widely reusable if they receive objects in this state rather than Borrowed.
See the next section for more details

As a Table, where LHS is current state, and Column is whether they have a specific
capability:

|            | Has alias | May alias | May move/disown |
|:----------:|:---------:|:---------:|:---------------:|
| Unowned    |   maybe   |    yes    |       no        |
| Owned      |    no     |    no     |       yes       |
| Borrowed   |   maybe*  |    no     |       no        |
| MaybeOwned |   maybe   |    no     |       no        |

\* This is a choice for the caller. If they opt in to stricter tracking, then
Borrowed cannot have an alias. See next section.

Described in terms of what parameters they may be passed to given that
the parameter is defined to be a specific state:

1. Unowned - Can be passed to Unowned or MaybeOwned. Whether it can pass to
borrowed depends on whether the caller opts in to stricter tracking. See the
next section for more details.
2. Owned - Can be passed to Borrowed or MaybeOwned, or may transfer ownership to Owned.
3. Borrowed - Can be passed to Borrowed or MaybeOwned.
4. MaybeOwned - Can be passed to MaybeOwned.

As a Table, where LHS is current state and Column is whether they can be passed
to a param of that state (or used as the $this in a method invocation,
when otherwise permitted):

|            | Unowned | Owned | Borrowed | MaybeOwned |
|:----------:|:-------:|:-----:|:--------:|:---------:|
| Unowned    |    yes  |   no  |  maybe*  |    yes    |
| Owned      |    no   | moved |   yes    |    yes    |
| Borrowed   |    no   |   no  |   yes    |    yes    |
| MaybeOwned |    no   |   no  |   no     |    yes    |

\* This is a choice for the caller. If they opt in to stricter tracking, then
Borrowed cannot have an alias. See next section.

#### Unowned and Borrowed

The choice of whether or not unowned may flow into borrowed is a subtle one -
one that developers will never have to consider under this proposal. The
usecase for not being able to pass unowned into borrowed is enabling of mutations
on linear objects (when they'd be banned on nonlinear ones).

Consider Rust. They have locally mutable values, which are analogous to our
Owned values, and `mut &`s and `&`s, respectively analoglous to our Borrowed and
MaybeOwned. The key is that rust does not have an unowned state - all values are
owned by a single reference. When a system requires linearity to enable
mutability, passing an unowned value into the equivilant to a `mut &` is obviously
unsafe, as that would result in mutations happening to nonlinear values.

Under this proposal, systems that require linearity for the purpose of tracked
mutations (such as Const Classes and Purity), would implicitly disallow passing
unowned values to parameters etc marked as borrowed. Developers would get an
error specifically about this telling them why and they will merely need to
conform to these restrictions, often via use of the MaybeOwned state.

One may note that this is in fact the main reason for the existance of the
otherwise quite similar Borrowed and MaybeOwned state. For systems that don't
require this stricter ruleset, those states are functionally identical (although
still not interchangable). For the most part, developers should not need to know
or care about this.

#### Concurrent uses of tracked objects

Borrowed, Owned, and MaybeOwned values may not be used multiple times
in the same statement or concurrent block as this creates implicit aliases,
breaking linearity. Further, objects of those states may not be passed to async
functions if they are not immediately awaited.

```
function takes_borrowed(borrowed Foo $b): void {...}
async function gen_takes_borrowed(borrowed Foo $b): Awaitable<void> {...}
function takes_two_borrowed(borrowed Foo $b, borrowed Foo $c): void {...}
function example(owned Foo $x): void {
  takes_borrowed($x); // fine
  takes_two_borrowed($x, $x); // banned
  // this might be fine because takes_borrowed($x) is finished before bar is called, but banned in V1 to be safe
  takes_two_borrowed(takes_borrowed($x), $x);
  takes_two_borrowed($x, takes_borrowed($x)); // as previous
  // as previous. This might be fine because foo($x) is run synchronously, but banned in V1 to be safe
  takes_two_borrowed(takes_borrowed($x), takes_borrowed($x));
  takes_two_borrowed(await gen_takes_borrowed($x), await gen_takes_borrowed($x)); // banned
  gen_takes_borrowed($x); // banned - must await immediately
}
```

For the above, any use of a local counts as a borrow and the same logic applies
for concurrent blocks as these individual statements. Note that invocations like
`foo($x)` cannot be returning `$x` internally as that would require aliasing a
borrowed value.

### Declaring Objects as Tracked

You will note below that there is no way to explicitly state that a value
is unowned. There is no usecase we could determine for which this was a useful
designation.

Note that only one of the following may be used per method object, parameter,
and return value, but multiple may occur in the same declaration header.

#### Parameters

Parameters may be declared to be any of Unowned, Owned, Borrowed, and MaybeOwned

```
public function foo(IFoo $foo): Foo { return new Foo(); }
public function foo(owned IFoo $foo): Foo { return new Foo(); }
public function foo(borrowed IFoo $foo): Foo { return new Foo(); }
public function foo(maybeowned IFoo $foo): Foo { return new Foo(); }
```

#### Return Types

Return values may be delared as either unowned or Owned

```
public function foo(IFoo $foo): Foo { return new Foo(); }
public function foo(IFoo $foo): owned Foo { return new Foo(); }
```

Returning a Borrowed/MaybeOwned value is forbidden, as doing so would create an
alias. Returning an owned value implicitly transfers ownership to the caller.

#### Method Objects

The object on which a method is called may be delared as Unowned, Borrowed, or MaybeOwned

```
public function foo(IFoo $foo): Foo { return new Foo(); }
public borrowed function foo(IFoo $foo): Foo { return new Foo(); }
public maybeowned function foo(IFoo $foo): Foo { return new Foo(); }
```

While it is logically sound for $this to be owned, with ownership transferred to
the method call, this is not permitted due to runtime considerations. There are
major issues with `$this` being available via things like stack traces rather
than just within the method itself. Further, unsetting `$this` is not something
the runtime can support at present.

### Moving Between States

#### Creating Owned Values

When an object is created it must explicitly opt in to the owned state by using
the `own` keyword, e.g. `$x = own new Foo();`. Any object that is not explicitly
created as such is unowned.

#### Transferring Owned Values

For those who have used C++ and Rust, transferring ownership will be somewhat familiar.

Consider the following functions, one that requires to be given an Owned value
and the other that creates one as above:

```
function takes_owned(owned Foo $foo): void {...}
function creates_owned(): void {
  $f = own new Foo();
  // would like to give ownership of $f to `takes_owned`
}
```

Ownership transferrance is done via the `move` keyword

```
function creates_owned(): void {
  $f = own new Foo();
  takes_owned(move $f);
}
```

In order to maintain linear ownership, after `move`ing a value, the variable
initially containing that value is unset (i.e. KindOfUninit). In the previous
example, attempting to use $f after invoking `takes_owned` would fail.
Additionally, as you would expect, attempting to `move` the same value twice in
the same statement/concurrent block is illegal.

Note that the `move` keyword isn't allowed on the result of an `own`
expression. Rather, it is always implicit when necessary e.g. in `takes_owned(own new Foo());`.

The other location of ownership transfer is at the return site of a function
marked as returning an owned value, as in the following

```
function returns_owned(): owned Foo {
  $f = own new Foo();
  return $f;
}
```

Note that the `move` keyword is not required here, as it is trivially determinable
from the signature of the enclosing function that an ownership transfer is required.

The `own` keyword may be used to receive an owned value from a function marked
as returning an owned value. Additionally, if the expression being returned is
one on which `own` can be used, `own` is also not required, and, in order to
remain consistent, disallowed:


```
function returns_owned(): owned Foo {
  return new Foo();
}
function returns_owned2(): owned Foo {
  return returns_owned();
}

function returns_foo(): owned Foo { return new Foo(); }
function creates_owned_foo(): void {
  $f = own returns_foo();
}
```

Note that it is possible to return an owned value in an async function. In order
to receive that owned value, the caller must immediately `await` and `own` the
result:

```
async function gen_returns_foo(IFoo $foo): owned Awaitable<Foo> { return new Foo(); }
async function gen_creates_owned_foo(): Awaitable<void> {
   $f = own await gen_returns_foo();
}
```

`Awaitable` is the only situation in which the ownership declaration does not apply
directly to the item following it. In this way, it is similar to `__Soft`.

Conditionally `move`ing values is currently disallowed. If, in a future version,
we change the `move`d variable to be null instead of unset, it is possible we
can allow this.

As a reminder, any value that _may_ be `own`ed but isn't is implicitly disowned.

#### Disowning Values

It is sometimes desired to disown a value midway through a function. This is done
via the `disown` keyword.

```
function foo(owned Foo $foo): void {
  // cannot alias $foo here
  $unowned_foo = disown $foo;
  // $unowned_foo is unowned. $foo is unset
}
```

Note that similarly to `move`ing a value, `disown`ing one unsets the
`disown`ed variable. In the same vein, a variable may not be conditionally disowned.

#### Linearly Lending Values

Owned values may be lent to other functions promising to maintain linearity, and
borrowed values may be lent to further functions. This is handled without
requiring additional keywords.

```
class Foo {
  public borrowed function borrow(): void {}
}

function foo(owned Foo $foo): void {
  borrow($foo);
  $foo->borrow();
}

function borrow(borrowed Foo $foo): void {
  borrow_again($foo);
  $foo->borrow();
}

function borrow_again(borrowed Foo $foo): void {}
```

### Additional information

#### Interactions with `null`

`null` is a valid value in any state, as it is not actually an object but rather
the lack of one.

#### Hierarchical requirements

Logically, the subtyping rules for the states can be thought of as follows,
where (|) is subtyping and ~~> is coercion:

```
     MaybeOwned

    /        \

Borrowed <~~ UnOwned

   |

 Owned
```

For simplicity, however, the initial implementation will require that
hierarchies be invariant in their usages of ownership states. It seems likely
that we can enable owned return positions to be subtypes of unmarked return
positions, but the rules for, and usefulness of, subtyping relationships for
parameters becomes less clear.

This includes the usage of the `__Unownable` and `__ReturnsBorrowedThis`
attributes, to be described below.

#### Properties of objects cannot be owned.

At present, properties (including container contents) are always unowned.

It is possible that this may be implementable in a future version.

#### Preservation of Single Ownership Type Per Variable Name

In order to enable performant tracking by HHVM, while retaining a reasonably
simple implementation within the emitter, variables may be used to reference
at most one ownership state. However, a single variable may be used to
reference multiple objects of the same ownership state.

```
class Foo {}
function my_function(
  owned Foo $owned,
  borrowed Foo $borrowed,
  maybeowned Foo $maybe,
  Foo $unowned,
): void {
  $owned = own new Foo(); // fine, still owned
  $disowned = disown $owned; // fine, new disowned variable
  $owned = own new Foo(); // fine, owned again
  $owned = $disowned; // error
  $borrowed = own new Foo(); // error
  $borrowed = $disowned; // error
  $maybe = own new Foo(); // error
  $maybe = $unowned // error;
  $unowned = own new Foo(); // error
  $owned = own new Foo(); // fine, still owned
  $disowned = own new Foo(); // error
  $disowned = new Foo(); // fine, creating implicitly unowned value
  $unowned = new Foo(); // fine, creating implicitly unowned value
  $new_unowned = new Foo(); // fine, creating implicitly unowned value
}
```

#### Capturing values in closures
Locals that contain Owned, Borrowed or MaybeOwned values cannot be captured as
that creates an implicit alias, breaking linearity

```
class Foo {
  public int $foo = 0;
  public borrowed function capturesMutable(): void {
    $x = () ==> { $this; }; // error
  }
  public maybeowned function capturesMaybeMutable(): void {
    $x = () ==> { $this; }; // error
  }
}
function captures_owned(
  owned Foo $obj1
  borrowed Foo $obj2,
  maybeowned Foo $obj3,
): void {
  $x = () ==> {
    // these are all errors
    $obj1;
    $obj2;
    $obj3;
  };
}
```

#### Ownership in Constructors

Constructors pose something of an unique problem for ownership tracking. In
order for tracking to work, one must be allowed to `own` the result of a
constructor call. However, if linearity is broken *within* the constructor, then
that no longer holds true.

The solution landed upon is for constructors to be opt-out for linearity instead
of opt-in. In the standard case, `$this` within a constructor is in the `borrowed`
state. Following the termination of the constructor, the rules apply as stated
above. The opt-out mechanism is via a new `__Unownable` attribute, which
indicates that the result of that constructor may not be `own`ed. Additionally,
hack errors triggered in a non-opted-out constructor will initially suggest
opting out as a last resort after other standard suggestions.

This conclusion resulted from analysis of patterns within the FB codebase.

Of the very large number of constructors, <3% of them would need to be opted out
if no further work was done. However, of that <3%, >95% of them are due to
invoking a helper function on $this which is not accepting of a borrowed value.
Upon inspection of a random sampling of those helper functions, a large
majority of them are trivial to allow accepting borrowed values with no other
changes (except for potentially marking a further function). Further, >4% of the
problematic constructors arise from capturing $this in a closure, which may
become legal in a future update (see Future possibilities section). Thus, only 1%
of the problematic cases are ones which require more serious refactoring. The
simplest solution for those are to convert them to a builder pattern in which the
builder function invokes the constructor and then does any problematic behaviour
following it.

We may choose in the future to remove this method of opting out.

#### Interactions with Memoize

Memoized functions may not return values as owned, as doing so would necessarily
allow creating multiple aliases to the same object. For objects implementing
`IMemoizeParam`, the `getInstanceKey` method must be valid to invoke on the
passed object for whichever state the parameter is marked. (e.g. if the param
is borrowed, `getInstanceKey` must use a Borrowed or MaybeOwned `this`);

Prior to rollout, we will determine whether this is actually necessary or if we
can globally enforce a MaybeOwned `this` for getInstanceKey methods

#### Interactions with inout

An `inout` parameter can neither be Borrowed nor MaybeOwned as these are
tantamount to returning such values, which is banned.

For simplicity, an `inout` parameter may not be `owned`. It is possible that a
future version of this feature will allow this if usecases arise.

#### Interactions with the Mutable Builder Pattern

The mutable builder pattern is one in which a class contains setters that modify
a property and then return $this. This can then be used as a chain to "build up"
the object. This becomes a problem when ownership tracking comes into play, as
if $this is tracked, then the return statement creates an alias.

The problem then becomes how to allow these objects to be built up in the case
where the object is tracked while not diminishing the experience for unowned
objects for general developers.

The solution to this problem for this proposal takes the form of a temporary
Hack. A function can be marked with the magic `__ReturnsBorrowedThis` attribute.
In the presence of this attribute, the return value can only be used by the
caller if the object the method is called on is unowned. Otherwise, the method
may be called, but the return value must be thrown away.

```
class Foo {
  private ?int $val;
  private ?int $val2;
  <<__ReturnsBorrowedThis>>
  public borrowed function set1(int $val): this {
     $this->val = $val;
     return $this;
  }
  <<__ReturnsBorrowedThis>>
  public borrowed function set2(int $val): this {
     $this->val2 = $val;
     return $this;
  }
}

function useit(): void {
  // this is the standard chaining version
  $unowned = new Foo();
  $unowned
     ->set1(42)
     ->set2(1234);

  // here, since return value cannot be used, must use multiple statements
  $owned = own new Foo();
  $owned->set1(42);
  $owned->set2(1234);
}
```

This is not ideal, but it is required for safety of tracking. We hope that in
the future additional features can be added to the language obviating the need
for this hack. See the discussion on void chaining linked below.

Note that methods with this attribute must act on a borrowed `this` and return
exactly `$this`.

## IDE experience

The new keywords will be supported in a first class manner with respect to syntax highlighing.

There will be a large number of new error types both in the typechecker and parser.

Due to the nature of this system, most developers won't see any errors due to
the inherent flexibility of the default state. When annotated code calls unannotated
code, we can straightforwardly suggest how to annotate the unannotated code to
facilitate progress. When annotated code calls annotated code, the error messages
will be similar to standard type errors where they will explain why the different
states are incompatible and it will be up to the user to fix them.


## Implementation details

### Interaction with other features

The initial version of the feature will have no interactions with other hack
features other than those explicitly expressed above.

Following the rollout of this feature, we expect to utilize it for sound
implementations of Purity, Const Classes, and Disposables. Note that the below
sections are examples rather than confirmed decisions. It is possible that
details about the interactions will change.

#### Disposables

Disposables currently have their own implemented version of ownership tracking
wherein disposables are "owned" by their using statement, and may be lent or
given to called functions. This is completely subsumed by this more complete
proposal, however they will also have additional restrictions (such as
prohibiting disowning them).

As an example, here is what currently exists

```

class MyDisposable implements IDisposable {
  public function __dispose(): void {}
}

<<__ReturnDisposable>>
function returns_disposable(): MyDisposable {
  using $foo = new MyDisposable();

  // error: Variable from 'using' clause may only be used as receiver in method
  // invocation or passed to function with <<__AcceptDisposable>> parameter
  // attribute Hack(4180)
  // return $foo;

  return new MyDisposable();
}

function accepts_disposable(<<__AcceptDisposable>> MyDisposable $md): void {
  // Parameter with <<__AcceptDisposable>> attribute may only be used as
  // receiver in method invocation or passed to another function with
  // <<__AcceptDisposable>> parameter attribute Hack(4188)
  // using ($md);

  // same error
  // return $md;
}

function takes_mixed(mixed $m): void {}

// Parameter with type 'MyDisposable' must not implement IDisposable
// or IAsyncDisposable. Please use <<__AcceptDisposable>> attribute or
// create disposable object with 'using' statement instead. Hack(4190)
// function illegal_param(MyDisposable $m): void {}

function example(): void {
  // Disposable objects may only be created in a 'using' statement or
  // 'return' from function marked <<__ReturnDisposable>> Hack(4187)
  // $x = returns_disposable();

  using ($x = returns_disposable()) {

    // Variable from 'using' clause may only be used as receiver in
    // method invocation or passed to function with <<__AcceptDisposable>>
    // parameter attribute Hack(4180)
    // takes_mixed($x);
  }

  // Variable $x is undefined, or not always defined Hack(2050)
  // echo $x;
}

```

and how it would look using ownership management (note that all error
messages are examples):

```
class MyDisposable implements IDisposable {
  // won't require `borrowed` on __dispose method because it's always the case.
  public function __dispose(): void {}
}

function returns_disposable(): owned MyDisposable {
  using $foo = new MyDisposable();
  // here $foo is borrowed

  // error: may not return borrowed values
  // objects in a using block are always borrowed
  // return $foo;

  return new MyDisposable();
}

function accepts_disposable(borrowed MyDisposable $md): void {
  // error: may not `use` borrowed values (or must `use` owned values)
  // using ($md);

  // error: may not return borrowed values
  // return $md;
}

function takes_mixed(mixed $m): void {}

// Disposable Parameters must always be Borrowed.
// function illegal_param(MyDisposable $m): void {}

function example(): void {
  // Disposable objects may not be disowned
  // returns_disposable returns an owned value
  // this value is implicitly disowned here
  // $x = returns_disposable();

  // Disposable objects may not be explicitly owned
  // they must be used by a `using` block
  // $x = own returns_disposable();

  using ($x = returns_disposable()) {
    // here $x is borrowed

    // cannot pass borrowed object to function expecting unowned
    // takes_mixed($x);

  } // here $x is destroyed following lending itself to __dispose

  // Variable $x is undefined, or not always defined Hack(2050)
  // echo $x;
}
```

However, when using ownership management it becomes possible to implement things
like potentially transferring ownership of disposables into called functions
that must then `use` them. There are a handful of more powerful features
required by the HSL IO library in order to make its implementation disposable.
Having them powered by ownership allows for these potential additions.

Given that they are never unowned, it is illegal to have a typehint of a
disposable type without an ownership annotation, typically `borrowed` or `owned`.

Conceptually, the object is "owned" by the using statement and is lent to the
block scope.

#### Purity

We're not going to go into too much detail on purity as it is still an
experimental feature, and we don't want to have discussions on its merits or
decisions at this location.

This is another example of the discussion above regarding requiring linearity
for the purpose of mutability.

In the experimental version, there are 4 states of mutability:
1. Immutable (the default) - an immutable nonlinear value
2. Owned Mutable -  an owned linear value that is mutable
3. Borrowed Mutable - a borrowed linear value that is mutable
4. Maybe Mutable - a value that may or may not be linear, so is immutable but also
must be treated as if it was linear.

The states of this proposal can replace the previous states 1-1:
Immutable -> unowned
Owned Mutable -> owned
Borrowed Mutable -> borrowed
Maybe Mutable -> MaybeOwned

If Purity is rewritten to use the ownership annotations, it must only add
the 2 rules, everything else directly falls out of this proposal.
1. Only Owned or Borrowed objects may be mutated.
2. Value types may always be mutated.

One might note that it's possible to pass an unowned value into the system
from impure code calling pure code, but that is no different than the current
state of the feature.

To be completely clear, this will completely replace the experimental mutability
annotations added by the reactivity project.

#### Const Classes

The design for const classes prior to this proposal worked fairly simply. They
can be modified within the constructor as well as from within private methods
called by the constructor, however once the constructor finishes, the classes
"lock" and become immutable.

However, this has a pretty unfortunate typehole:

```
const class MyConstClass {
  private int $x = 0;
  public function __construct() {
    $this->can_mutate();
  }

  private function can_mutate(): void {
    $this->x = 42;
  }

  private function my_unrelated_method(): void {
    // do stuff
  }

  public function oops(): void {
    $this->my_unrelated_method();
    $this->can_mutate(); // uhh....
  }
}

function example(): void {
  $cc = new MyConstClass();
  $cc->oops(); // explode
}
```

This problem is solvable via a combination of ownership management and making
const classes more powerful. The solution is simple: opt const classes into
the stricter ownership tracking and making them immutable when unowned.

```
const class MyConstClass {
  private int $x = 0;
  public int $even_more = 42;
  public function __construct() {
    $this->can_mutate();
  }

  private borrowed function can_mutate(): void {
    $this->x = 42;
  }

  private function my_unrelated_method(): void {
    // do stuff
  }

  public function oops(): void {
    $this->my_unrelated_method();
    // error: cannot call function with borrowed $this
    // $this here is unowned
    // $this->can_mutate();
  }
}

function example(): void {
  $ncc = own new MyConstClass();
  $ncc->even_more++;
  $cc = disown $cc;
  $cc->oops(); // safe!
}
```

In this system, all instances or typehints of a const class will be opted-in to
the stricter tracking. Further, const classes can now make use of the
extended constructor pattern!

### HHVM Support

This feature will have full HHVM support and enforcement. Objects may only be
used as described above or else they will throw an exception. For cross-function
boundaries the enforcement will work similarly to type enforcement.

### Codemodding

The only major codemod required for this feature is the interaction with
constructors described above to insert the `__Unownable` attribute where
necessary.

Other than that, no codemodding for this will be done except to convert
the current Disposables and temporary Mutability tracking feature to this
complete one. In that case, we will first do a static transformation from the
old attributes to the new system to confirm everything still typechecks.

## Revert Plan

If, following rollout of this feature, we decide to remove it, reversion
will simply be a codemodded removal of the associated keywords. However,
if used within Purity, Const classes, and other major features, it becomes
increasingly difficult to roll back without exposing major soundness holes
in the language.

Note, however, that the initial plan is to have the system implicitly back both
disposables and the experimental Reactivity/Purity syntax under the hood,
so we should hopefully know prior to full rollout if revert is necessary.


## Design rationale and alternatives

This design was created under a few basic principles:
1. Opt-in feeling: It is important for a large codebase that this need not be the default
2. Usability: The states are not conceptually difficult and hack-errors can
be used to direct newcomers towards correct patterns, as situations of the
form "You are doing A, which is wrong, but B would be OK" are common.
3. HHVM Enforceability and efficiency: In order to be used for optimizations,
HHVM must be confident of the guarantees. Therefore, it must be enforceable.
Therefore, it must be efficient.

This is a completely new segment of the language such that attempting to do this
via existing features is a nonstarter.

Not doing this prevents multiple projects, either via soundness issues or simply
lack of required featureset. See above for examples.

### Alternative designs

There are multiple minor tweaks considered to the above syntax.

Instead of `public borrowed function foo(): void {...}`, we considered
`public function foo(): void where this is borrowed {...}`.
We decided against this as, while it makes it clear we're referring to `this`,
it requires modification of function-level where clauses and is extremely verbose.
It is worthwhile to note that `static` appears in the same location and is a modifier
that describes the state of `$this`

We considered the keyword `owned` instead of `own` for marking a value as
owned, but don't believe the extra two letters add anything.

We considered referring to the unowned state as disowned, but this is
confusing for things that are never owned.

As an alternative to `__ReturnsBorrowedThis`, we would like to resurrect the
[void chaining](https://fb.workplace.com/groups/1353587738066495/permalink/1690769081015024/) discussion.
This allows us to use the fluid builder pattern on non-unowned code, but also
allows for the potential to completely remove the pattern of functions that
return $this for the purpose of chaining.

An alternative to the `disown` keyword is to use `release`. Bikeshedding welcome.

The __ReturnsBorrowedThis attribute is technically unecessary as the emitter
can infer when it would be required and handle the situation appropriately. We
think it's better to be explicit here.

When owning the result of an async function, should the `own` be before or
after the `await`? We landed on before because you're `own`ing the wrapped value,
not the Awaitable. Maybe there are arguments for the alternative?

## Drawbacks

It is complicated and requires significant runtime and typechecker support.

## Prior art

At present, there is only one major language implementing an ownership system.
There are a handful of languages implementing Linearity systems, however that
does not map particularly well to an imperitive language like our own.

Rust's ownership tracking is more robust and allows for nested ownership
tracking (such as owned properties) and explicit lifetimes. This is made
possible as they have an AoT compiler that can do enforcement and analysis. We
have different constraints. Further, we do not require quite the same complexity.
This can more simply be described as `borrowed T -> &mut T` and `own T -> T`.
However, rust doesn't typically allow for unowned values. The closes it comes
is `&T` immutable references. These are references to an owned value that is
merely being utilized multiple times at once (and so must be immutable).
In general, our requirements are flipped. They want everything owned. We want
most things unowned with specific things being owned.

Additionally, there a handful of other languages actively exporing this space:

[Swift's ownership manifesto](https://github.com/apple/swift/blob/master/docs/OwnershipManifesto.md)
is designed based on different constraints and purposes. Ours is mostly to unblock other features,
while also enabling some performance gains. Their purposes are flipped.

[Haskel](https://github.com/ghc-proposals/ghc-proposals/blob/master/proposals/0111-linear-types.rst#id22)
with [tech talk](https://www.youtube.com/watch?v=o0z-qlb5xbI&t=1s):
Rather than discussing linearity of values, they describe linearity of functions,
such that for a fn a -> b, if b is consumed exactly once, then a is consumed
exactly once. They implement polymorphism by taking in closures and making the
overall linearity of the function be polymorphic upon the linearity of the
passed function. Further, they do not have the concept of borrowing linear
values, which is extremely important to our system.

[Clean](https://clean.cs.ru.nl/download/html_report/CleanRep.2.2_11.htm):
Clean has a similar system to haskell except that it's type-based rather than
function based. It has some similar properties to our system, but considers
non-unique <: unique because of its functional nature, such that you can
pass uniq values into a function accepting non-uniq, but what you get out is
nonuniq. In general, these all apply to consumed inputs, thus they don't have
the borrowed state. They do have a generic on types to specify propogation
of uniqness, but again, this requires the move in -> move out scenario.

[Idris](http://docs.idris-lang.org/en/latest/reference/uniqueness-types.html):
They use dependent typing for this and have discovered the need for a borrowed
state but have not yet handled the polymorphic MaybeOwned state. They have a
representation for Owned V Unowned, but specifically note that it doesn't allow
for representation of borrowed. It's more similar to haskel's polymorphism due
to its lack of a borrowed state.

Also, from another paper or note:
> Uniqueness types, in Idris, are being replaced by linear types

#### Papers on the subject:

[Islands: aliasing protection in object-oriented languages](https://dl.acm.org/doi/10.1145/117954.117975)
(by John Hogg) (paper available upon request):

The overall goal here is to create "islands" of objects in which they are, as a
whole, only referenced via a single "bridge" such that all modifications to
the island (or even access to it) must go through the bridge. Thus, they may
handle things like side effects, which are nonissues for const objects and
disposables, and general alias management and concept encapsulation.

Three major concepts: `read` `unique` and `free` where `read` is similar to const,
and the other two are about whether aliases are allowed.
> A variable z many be unrestricted, read, unique, free, read and unique, or read and free.

Interestingly, here 'unique' means that there is only one reference to an object
via the heap (such as instance properties), and the rest are stack-based, and
`free` means there are no heap-based references to the object (even if the
object itself is on the heap).

A unique object is most often used via lending it out on the stack, similar to
our `borrowed` parameters. They don't need two keywords because `unique` appears
on things like instance properties to imply its the only heap-based reference
and on parameters to mean that it's a reference to a `unique` object. However,
in many ways, their `free` is similar to our `owned`. You can create a `free`
reference by destructively reading a `unique` heap-based reference. Further,
`free` references can only be accessed via destructive reads (creating another
`free` refence or assigned into an `unique` one).

I honestly, don't understand this restriction and description.
> If a method receiver is unique, then every parameter and the result must be
> read or unique or free.
> Unique is a transitive property. If an object’s acquaintances are aliased,
> then the observable state of the object may be unexpectedly changed, even if
> there is no aliasing of the object itself. Therefore, an object being accessed
> as unique must not import or export any unprotected references. An unprotected
> parameter could be retained by the object, however.

This doesn't apply to `free` receivers, because it doesn't matter.

This is an interesting approach for allowing tracking of members. However, this
doesn't solve our problem with the `unique` object being referenced from within
an `unrestricted` object, which they don't talk about as far as I can tell.

As a side note, they don't cover practical problems of code deduplication and/or
polymorphic states, which are the main problems we generally struggle with.
They do generally seem to have an opt-in system, however, in that that outside
an "island" everything can just be unrestricted, and only when interacting with
an island must things be annotated.

[Alias burying: Unique variables without destructive reads](http://www.cs.uwm.edu/faculty/boyland/papers/unique-preprint.ps) (by Boyland)

> Alias burying: when a unique field is read, any existing aliases are
> required to be dead. Together with restrictions on aliasing across procedure
> calls, alias burying can be checked by a static analysis in a modular fashion.

Their main goal is to be able to track mutations for safe concurrent reads, which,
while not identical to our goals, is more or less translatable.

They concur on our decision to mark misbehaved constructors
> A constructor can keep a reference to the object it is creating in a globally
> accessible location; not all constructors create unshared objects. Thus a
> constructor that does create an unshared object should be declared as such.
> ...
> Constructors in Java are not responsible for creating an object, just
> initializing it. Therefore constructors that “return” unique objects are
> annotated as having borrowed receiver

Their system of ownership and borrowing seems *very* similar to our own
> An alias of a variable for our purposes is a variable that refers to the same
> object as the first variable. This definition is nonstandard, but is convenient
> for languages such as Java that do not have explicit dereferencing
> A static alias is a field variable; a dynamic alias is a receiver, parameter,
> local variable, or return value alias
> A dynamic alias of an otherwise unique variable is also known as a “non-consumable,”
> “uncaptured,” or “limited” variable.
> Such a variable may be used for computation and copied into other local
> variables or parameters (which become dynamic aliases) but must never be stored in a
> field. In this paper, we refer to variables whose values may never be stored
> (or returned) as borrowed variables.

They recommend using `borrowed` to refer to a value that may either by a dynamic alias
to an owned object or an unowned object.

However, they further recognize that the existance of a union state of this
description results in issues with tracking mutations.
> Weakening a uniqueness variant to admit dynamic aliases makes it easier to
> program with unique variables than with destructive reads alone, but this
> weakening comes with a cost. One main purpose in avoiding aliasing was to make
> it easier to isolate mutations; dynamic aliases make it harder to track down
> all mutations.

They allow for unique static aliases (our borrowed) similarly to Hogg, above.

In general, their states are Unique, Unrestricted, and Borrowed. As far as I can
tell this works in a system that cares only about aliasing and not restricting mutations to
linear values because you don't need dynamic aliases that are required to be references
to unique objects. They allow modifications to Borrowed objects. This seems to
generally safe only because they mark mutating methods as `synchronized`, which
alleviates any interleaving concerns automagically. I'm not quite clear on how
that works, however, given tht reads can happen in nonsynchronized methods.

> When a unique field of an object is read, all aliases of the field are made undefined.
> If we can determine statically that no alias is ‘buried alive’, no dynamic
> checks will be necessary.

Their goal is to statically prove that this invariant holds true such that
unique properties _must_ actually be unique dynamically. We prove that via a
combination of static and dynamic checks (because we can't trust types, therefor
disallowing us to trust that function invocations comply).

Their hiearchy of states is Unique/Owned <: Owned/Shared <: Borrowed/Shared <: undefined

Unique/Owned: only allows for explicitly unique values and they must be moved in
destructively.

Owned/Shared: allows for either unique or shared values and generally can treat
them as shared. After passing a unique value to this, one can no longer presume
uniqueness

Borrowed/Shared: May be unique or shared, but my not generate new aliases except
to pass to other borrowedly notated states.

> A shared (that is, not unique) parameter may be declared borrowed

This sentence in isolation is somewhat inscrutable. Based on context and later
descriptions, I believe this is noting that a shared value may pass into a parameter
declared as accepting borrowed. This seems *potentially* fine considering it's
specificalyl stricter, but it doesn't allow making any assumptions about borrowed parameters

> a unique value may be passed to a procedure expecting a shared parameter, but
> a borrowed value cannot be

This explicitly allows for passing unique values into params expecting shared.
This seems like a tenuous assertion if you want to statically prove unique
objects aren't leaking refs.

They further note that
> The analysis also uses an annotation on procedures that indicates which variables it may read

but then never mention this again. I believe this is specifically related to
fields on passed in objects.

> The interface annotations may be read as obligations and dual privileges:
>
> **unique parameters** The caller of a procedure must ensure that parameters declared unique are indeed
> unaliased. A procedure may assume unique parameters are unaliased.
>
> **borrowed parameters** A procedure must ensure that a borrowed parameter is not further aliased when
> it returns. A caller may assume that a borrowed actual parameter will not be further aliased when
> the procedure returns.
>
> **unique return values** A procedure must ensure that a return value declared unique is indeed
> unaliased. A caller may assume that a unique return value is unaliased.
>
> **unique on proc entry** The caller of a procedure must ensure that any unique field read or written by
> the procedure (or by a procedure it calls, recursively) is unaliased at the point of call. A procedure
> may assume that the first time it reads or writes a unique field, the field has no (live) aliases.
>
> **unique on proc exit** A procedure must ensure that any unique field it reads or writes during its
> execution is unaliased upon procedure exit, and that no fields have been made undefined. A
> caller may assume that a procedure does not create any aliases of unique fields or make fields
> undefined
>
> If all procedures fulfill these responsibilities, then the only potentially live variables that may be made
> undefined due to alias burying in a procedure are variables set by this procedure. Thus assuming we
> can check a procedure, we can check a program

From what I can tell, the first three have to do with annotations on params/return values
and the last 2 are just general invariants all methods must adhere to. How they
assert that this magically allows for static assertion of correctness is beyond me.
They certainly don't explain it.

In general, their restrictions on unique and borrowed are identical to ours.
Having their shared state be a supertype of unique and a subtype of borrowed is
somewhat baffling.

If we consider our lack of unique member variables, the last two restrictions
become moot, but the ability to pass unique references to effectively unrestricted
params accepting shared values is confusing.

> Of course, accessing unique fields of possibly shared objects must still be properly
> synchronized in order to prevent race conditions. When the lock is released,
> the unique field must be uncompromised and have no live aliases. If a language
> has asynchronous calls (Java does not), then we must add the rule that
> borrowed values cannot be passed in such calls

Not accepting borrowed inside async calls is an interesting concept, but seriously
limits the usability in our system.

All in all, this system strongly resembles our own, but takes a hard left turn
at a crucial fork and ends up in a place that doesn't quite solve our problems

[Linear Haskell: Practical Linearity in a Higher-Order Polymorphic Language](https://arxiv.org/pdf/1710.09756.pdf)

A relevant quote:
> Seen as a system of constraints, uniqueness typing is a non-aliasing analysis
> while linear typing provides a cardinality analysis....The former aims at
> in-place updates and related optimisations, the latter at inlining and fusion.

In general, their system does not neatly correlate to our needs for the above
reasons.

[Promises: Limited specifications for analysis and manipulation](https://courses.cs.washington.edu/courses/cse590p/00wi/promises.pdf)
(By Chan, Boyland, Scherlis)

Their system of uniqueness management generally seems to mirror our own.
They consider both the general concept of unique references and the issue of
ill-behaved constructors before noting that being able to pass unique references
to parameters that promise not to store lasting aliases do not cause additional
problems. They refer to these as `limited`.

They, too, consider `limited` to be a promise rather than a requirement.
This is contrasted with `unique` parameters that *require* the object passed in
to be unique (and consumed upon passing). This is similar to the concepts by
Boyland in his above paper (which actually came later than this one).

> Since the caller’s copy of the reference cannot be accessed during the dynamic
> context of the call, one can pass a unique reference as a limited unique
> reference instead (as long as the reference isn’t passed as more than that one parameter).
> Limited unique parameters are unaliased within a certain dynamic context and
> have the properties of both unique and limited parameters

Their `unique` is obviously analogous to our `owned`. limited` is generally
analogous to our MaybeOwned with `limited unique` being their borrowed.

They additionally consider the possibility of storing unique values within other objects:

> `Unshared fields` are really no more than a form of unique variable, except
> allocated in the store as part of an object. Similarly static unshared fields
> hold unique references associated with the class object.

> A problem with unshared fields is that they invalidate the assumption that a
> variable, and thus the reference it contains, is only accessible from a single
> frame. For instance, suppose that within a class with an unshared field, we
> call a method with the reference from this field as a parameter. Since there
> is no guarantee that the object with the unshared field is itself unique, we
> cannot assume the field will be inaccessible during the call.

> A simple solution to this is to use a promise that guarantees that the code
> being called will not access that field. Thus, we only allow an unshared field
> to be an actual parameter if it is passed as limited and the method to which
> it is passed does not read or modify the region for that field. As a
> consequence, if we have existing code that does need to access that region, we
> may have to nullify the unshared field during the call.

All in all, their description of this system is given in relatively short order,
but is quite analogous to our own.

[Deny Capabilities for Safe, Fast Actors](https://www.ponylang.io/media/papers/fast-cheap.pdf)

The overall goal of this system is to allow staticly checkable safe shared memory.
Their system is interesting. It is effectively a matrix of whether
local/global/both aliases are allow to read and/or write to a reference.

They make the interesting choice to note receiver state between the fn keyword and the fn name.

This seems unenforceable statically (box being disallows global writes):
> If an actor has a box reference to an object, no alias can be used by other
> actors to write to that object. This means that other actors may be able to
> read the object, and aliases in the same actor may be able to write to it
> (although not both: if the actor can write to the object, other actors cannot
> read from it)

Like us, they separate between methods that can be run asynchornously and ones that don't.
It's not safe to give a `box` to a method that can run asynchronously, because the
caller can retain a locally mutable instance, so the async method could read while
the caller does some writes. We ban this via strict limits on borrowed objects
in concurrently executed statements.

They note that passing an owned value to another async method is safe because it
is destroyed in the caller.

They suggest that `tag` (all aliases allowed) cannot be read/written, but may have
further async methods invoked upon them. Those async methods may see `this` as
`ref` (no global aliases, all local aliases). They consider `tag` objects to be
`actor`s (the objects on which async methods are invoked). I don't understand
why this distinction makes mutating their inner state safe.

Apparently you can store tag reference to an otherwise iso (global deny, local allow) reference
> a newly created alias of an iso reference must be neither readable nor writeable (i.e. a tag).

At this point, I decided that the system described was significantly more complicated
than our own with questionable applications to our problemspace. I skimmed the rest
of the paper and determined that I was correct.

[Ownership types for flexible alias protection](http://janvitek.org/pubs/ecoop98.pdf) (by Clarke, Vitek, Potter, Noble)
(this won "the most influential paper of the last decade" at OOPSLA'16)

The overall goal of this paper is to statically ensure abstraction implementation
encapsulation such that given an abstraction A exposed via an API on object B
implemented on classes C...N, the internals of the implementation of the
abstraction A via C...N cannot be referenced outside of B and the API must
be utilized to realize the abstraction.

They note that value types don't require tracking because they're immutable (or
in our cases, sometimes COW).

They suggest the concept of splitting the workings of a container into two pieces:
mutable state that is visible only internally, and immutable state that is
fine to expose (and even potentially share between multiple containers).

It is further possible that for nested containers, the exposed immutable state
may actually be the inner mutable state for an inclosing container. In this way,
it is noted that the immutabile set of an object is only immutable to that object,
but not necessarily to the outside.

They consider the concept of "roles", which map to the different set types. This
is used for the purpose of more abstractly separating (and keeping separate) of sets.

An important quote that also applies to our own system:
> Note that although they are similar, aliasing mode checking and type checking
> are completely orthogonal. An expression’s aliasing mode correctness implies
> nothing about its type correctness, and vice versa.

> The aliasing mode system comprises the following modes: arg, rep, free, var,
> and val.

These modes are/work as follows:

arg: The state visible externaly.
> only provide access to the immutable interface of the objects to which they
> refer. There are no restrictions upon the transfer or use of arg expressions
> around a program

rep: The state inivisible externally.
> Can change and be changed, can be stored and retrieved from internal containers
> but can never be exported from the object to which they belong

free: Objects with a reference count of exactly 1 (such as recently created objects).

var: "provides a loophole for auxiliary objects which provide weaker aliasing guarantees"
Works like the default symantics of non-alias checking languages except for the
assignment compatibility constraints.
> a mutable object which may be aliased. Expressions with mode var may be changed
> freely, may change asynchronously, and can be passed into or returned from
> messages sent to objects

val: Value types.
The following is because their value types are truly immutable rather than COW:
> has the same semantics as the arg mode, however, we have introduced a separate
> val mode so that explicit arg roles are not required for value types

Arg and Rep modes can be further tagged with Roles to ensure that two object
of the same mode but designed for different purposes cannot intermingle.

None of these modes are assignment compatible except for `free` which can be
used for any of the others.

They note that depending on the overall API of an object, it may or may not be
aliased in different ways.
> For example, if an object uses only modes arg, free, and val, it will be a
> “clean” immutable object, that is, it will implement a referentially transparent
> value type. If all of an object’s method’s parameters and return values (except
> the implicit self parameter) are restricted to arg, free, or val, the object will be
> an alias-protected container with flexible alias protection, and if in addition it
> has no variables of arg mode, the object will provide full alias encapsulation.

All in all, this is an interesting system, but one that is designed around
different and incompatible restraints from our own. This can potentially be
mimicked via something like considering pure contexts rather than pure functions,
but that doesn't allow for the solutions to other language requirements.

[Gradual Ownership Types](https://ilyasergey.net/papers/gradual-esop12.pdf) (By Ilya Sergey and Dave Clarke)

The overall goal of this paper is to explain a system by which ownership annotations
can incrementally be added to a system, similarly to how an untyped program can have
types added to it incrementally.

They focus primarily on the situations involving nested ownership, which results
in being less applicable to our usecase. Further, their goals are mostly related
to proving soundness of the overall system. They spend the vast majority of the
paper laying out the precise proofs and mathematical properties of the system,
which has less obvious applications to us. It might be useful to have someone
with a better theoretical background take a look at this.

[An object-oriented effects system](https://www.researchgate.net/publication/221496521_An_Object-Oriented_Effects_System) (By Greenhouse and Boyland)

The overall goal of this paper is to describe a system by which two method calls
can ensure that neither mutates state available to the other, allowing things
like better interleaving. This is an obvious corralary with our goals in doing
ownership tracking to ensure sound purity enforcement.

They define "regions", which can be thought of as similar to namespaces except that
they are defined within classes and their contents can be one or more properties.
When defining a property, the region they are within is specified. Lack of specification
implies the `Instance` region, which lives inside the global `All` region.
They also include the concept of static regions containing static members (and
doesn't live inside the Instance region). Following this, methods specify which
regions (if any) they act (read and/or write) on. The typechecker guarentees
both that these are true statements locally and that they are transitively true.
Overall, this is effectively another layer of privat/public/protected specifiers
that typically exist for more fine control over encapsulation layers.

They additionally allow for members to be marked as `unshared`, stating that
referring to the object requires going through that member, as well as `unique`
on parameters and return values, allowing for ensuring that those objects
have a ref-count of 1. `Unshared` is entirely used for the purpose of promising
that internals are not shared. They note that `Unique` and newly created objects
don't require effects tracking due to their linearity.

All in all, this system is powerful and fascinating, but doesn't handle issues
arising from dealing with the relative capabilities of non-members (such as
parameters) in a way that is ergonomic.

I can certainly see such a system being generally beneficial for the purpose of
encapsulation of different concepts described within members of a class, but for
the purpose of generalized ownership/uniqueness (and the benefits it yields), this
doesn't satisfy our constraints.

[Capabilities for uniqueness and borrowing](https://link.springer.com/content/pdf/10.1007%2F978-3-642-14107-2.pdf) (By Haller and Odersky)

[Kilim: Isolation-typed actors for java](https://www.malhar.net/sriram/kilim/kilim_ecoop08.pdf) (By Srinivasan and Mycroft)

[Uniqueness and reference immutability for safe parallelism](https://www.cs.drexel.edu/~csgordon/papers/oopsla12.pdf) (By Gordon, Parkinson, Parsons, Bromfield, and Duffy)

[Copying, sharing, and aliasing](https://www.researchgate.net/publication/2821935_Copying_Sharing_and_Aliasing) (By Grogono and Chalin)

## Unresolved Questions

Do we want to elide `own` in return statements as described above or we rather
it be explicit? e.g. in a function that is marked as returning owned, do we have
`return own new Foo()` or `return new Foo()`? Note this also applies if instead
of `new Foo()` we were invoking another function that returning an owned value.

For the initial rollout, should we only allow usage of this feature in the
precense of cases that require the feature (i.e. const classes, disposables, and Pure/Rx)?
This would ensure a tighter rollout that wouldn't affect quite as many
developers.

Would we want something like class-level where clauses to mark that *all* methods
on a class required, for example, borrowed `$this`?

Is `own $f = new Foo();` a viable alternative for creating a newly owned value?
The major problems are for things like this:
`function foo(own Foo $f): void { }`
`foo(own new Foo());`
We could potentially elide `own` on constructed objects being passed directly
to things requiring an owned/borrowed/etc object. However, this doesn't work
quite well for functions that return owned values.

Should we require specifically that params etc with ownership notations have
types (in the runtime)?

Should we require (in hack) that the types of params etc with ownership notations
be a class or interface? (this would be changed if we want to track value types)
Should we ban using ownership notations with mixed, dynamic, unbounded generics, etc?

### The general question of naming

Suggestion: rename "Borrowed" to "Uniquely Borrowed", or "Unaliased"

Suggestion: rename "Unowned" to "Shared" as it's similar to C++ use cases for
shared_ptr.

Suggestion: rename "MaybeOwned"  to "Restricted".
```
function foo(restricted Foo $f): void {}
public restricted function foo(): void {}
```
From the user perspective, it is nonobvious what restricted means here.
Further, we can imagine that restricted is a bit too vague considering
potential restrictions other than ownership.

Suggestion: rename "UnKnowed" to '_' or '_&'.
MaybeOwned feels similar in concept to a polymorphic type/generic, and it works
out to being roughly the same as a "top" type.
The problem for this is similar to the above reasoning for restricted. Without
context, it's unclear what this refers to.

Suggestion: rename "Unowned" to "NonLinear". Looking at the hierarchy, it's
clearly not linear and "MaybeOwned" behaves like a wildcard type.
Then we have the following taxonomy:
Linear: Borrowed, Owned
Nonlinear: Unowned->NonLinear
Either (exclusive): MaybeOwned

Should we generally re-describe this from `ownership` to `uniqueness`?
Something like
Old -> New, Keyword
Owned -> uniq, `uniq`
Borrowed -> uniq ref,  `uniq&` / `&uniq`
MaybeOwned -> maybe uniq ref, `uniq?&` / `?uniq&` / `?&uniq` etc
unowned -> nonuniq ref, still no keyword
with the state-changing operations being `move`, `uniq`, and `share`.
The new version of MaybeOwned feels a bit soupy. Maybe just `?uniq` or `uniq?`?

Suggestion: unowned -> Shared, Owned -> Uniq, MaybeOwned -> MaybeUniq, Borrowed -> Borrowed

Suggestion: MaybeOwned -> top/mixedref?

## Future possibilities

Mostly discussed in the interaction section above.

We may want to allow lambdas to close over owned/borrowed values as long as they
are specially marked and themselves treated as borrowed values.

```
function foobar(vec<int> $v): vec<int> {
  $f = own new Foo();
  return Vec\filter($v, borrowed ($i) ==> $i < $f->num);
}
```

Potentially relax some restrictions on using borrowed/owned values multiple
times in the same statement if/when we can prove soundness.

Could we have some way of marking an arbitrary class/hierarchy as required to
always be linear? Useful for things like disposables but also generators,
where trying to iterate them more than once goes badly. Maybe promote to keyword later?

Should we have an annotation to generally opt-in whole functions to the
stricter tracking for those who want to utilize the stricter system? This would
probably utilize the coeffects system to enforce transitivity

Hack Native has specifically requested we don't close the door on the possibility
of utilizing this system to globally track Arrays and strings in order to avoid
the need for refcounting them. The major blocker for this is that for that to
work, those types would need to *never* be unowned, which would result in a huge
usability degredation.

### Deeper Tracking

We could potentially allow properties of objects to be tracked as well. This
area has not been too deeply explored due to our lack of obvious need for it.
However, were we to add this, here are some considerations:

Some options:
1. We'd have to do something special when you disown the containing object such
as recursively disowning any properties.
2. Ban disowning such objects (linear forever)
3. When disowning objects, if they have any props marked as containing owned,
assert they're null or throw. That would work in the runtime but it would be
difficult for the typechecker to enforce (since it would have to keep track of
if every owned prop is null or not).

Note that we'd have to extend the "passing multiple refs" rule to include
referencing a prop and the outer object at the same time.

## FAQ

There are a handful of questions that don't fit in the above HIP, but were
themes among feedback received.

**Q** Should we consider making the unowned -> borrowed conversion opt-in via an
<<__AllowUnowned>> attribute on the function decl header? This makes the
system strict _unless_ explicitly opted out.

**A** This makes the system more complicated and is probably the wrong default.

**Q** Do we need a __Soft equivilant?

**A** Since unowned can flow into most things, a __Soft doesn't buy much.

**Q** HHVM can produce optimizations based on the guarantees described below when
utilizing these features. Can I break these guarantees with HH_FIXME, typeholes,
or abusing erased generics? If so, what happens?

**A** No. While we're likely to require types while using this feature, the
assertions made here are fully enforced by the runtime. The ownership concept
is fully reified regardless of the static types themselves.

**Q** what guarantees are we trying to provide via ownership system, and
specifically the "borrowed" state? More specifically, how does this apply when
mixing normal objects and disposables?

**A** A combination of strong and weak lifetime guarentees. The important piece is
that these objects don't get mixed. Const classes and disposables are
perma-opted-in to stricter tracking, so they get "strong" lifetime guarentees.
Further, anything in the `owned` state has a strong guarentee. The only weak
guarentee comes into play with the borrowed state, since that can actually be
nonlinear, but the guarentee there is from that point on it will stay linear.

**Q** can `own` be implied by `new` or is the `own` keyword just there for clarity?

**A** no. Not all created objects are tracked and owned. Many objects are never
logically owned.

**Q** why `$f = own new Foo();` instead of `own $f = new Foo();`?

**A** The problem comes into play for things like this:
```
function foo(own Foo $f): void { }
foo(own new Foo());
```
For runtime enforcement, we need to correctly forward `owned`ness across
function boundaries. This may be possible in the case of an explicit `new`,
but becomes even more complicated in the presence of functions returning owned

**Q** Can you disown a value received from a function invocation? Would there be any
advantage to doing something like that rather than let it implicitly become unowned?

**A** You cannot, and at present there isn't an advantage.

**Q** Will most code need to use these annotations?

**A** We generally expect the majority of code not to need to deal with ownership at all.

**Q** Why would a developer want to use this feature? What patterns should they
look for in their code that will make them want to track ownership?

**A** We don't expect most developers to want to generally opt in their codebase.
We give most of the "worry free" parallelism rust gives by using an
asynchronous model rather a true parallel one. There will likely be rare cases
where users want to maintain linearity for the lifetime of an object, in which
case this can be used to give them type-and-runtime level guarentees. The most
likely scenario for use will be very hot code that uses these annotations to
tell the runtime that it may safely elide reference counts and do other
optimizations.

**Q** Would the design be different if we had types in compilation? What would be
different? Would it require fewer annotations to the code?

**A** Types in compilation enable a handful of more powerful features such as more
deep tracking. However, as with typing, we would still need the declarations to
contain the developer intent. It is possible that some of the function-local
restrictions such as single-flavour-per-local could be relaxed. It is also
possible that more inference about the function-local state of objects could
be inferred rather than requiring explicit owning and disowning. See below
about lambdas.

**Q** Same question as previous but with just declaration level information

**A** Declarations in compilation unfortunately do not yield much useful information
for this feature. We could be smarter about handling named top-level functions,
but that doesn't buy us much.

**Q** How should I think about this ownership system. It isn't a type right? So
it wouldn't make sense to write say `type OwnedC = owned C`? How about
`vec<owned C>`?

**A** ownership is conceptually a parallel concept to typing in most cases. Ownership
is an attribute of an object the same way the type is, but ownership state isn't
a type. It's possible we could implement aliases, but because we rely strongly
on compile-time guarentees for efficiency, that has not been investigated for
the first iteration of this feature. `vec<owned C>` doesn't make sense because
we only have shallow ownership tracking. If we some day allow for deep tracking,
we will likely revisit this.

**Q** How will this interact with lambdas? Will ownership annotations need to be
placed on parameters? Can they be inferred?

**A** Ownership annotations will be required. From the perspective of this feature,
they are equivilant to any other function declaration. Types in compilation may
allow us to avoid this requirement.

**Q** How does this relate to projects like Co-Effects?

**A** This is mostly orthogal to Co-effects. There will likely be some co-effects
that add/remove capabilities based on ownership state (such as mutability in
pure code), but otherwise one does not depend on the other. Co-effects are about
function-level contexts and ownership tracking is object/instance/variable specific.
