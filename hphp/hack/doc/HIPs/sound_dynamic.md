# HIP: Sound `dynamic` type

## Motivation

The `dynamic` type was created to type legacy PHP code where types are optional:

```
// before
function f($a) { return 3; }
// after
function f(dynamic $a): dynamic { return 3; }
```
The dynamic type admits any operation (subscripting, member access, etc.) and
does not have any direct subtypes or supertypes beyond `nothing` and `mixed`.
Its semantics are defined by a non-transitive relation called *coercion*. Any
type can coerce to dynamic at enforcement points, and dynamic is allowed to
coerce to types that are enforced at runtime:
```
function f(dynamic $d): int { return $d; /* ok */ }
```

Unfortunately, coercion to dynamic is unsound for arbitrary values. Consider a
simple generic Box class:

```
class Box<T> {
  public function __construct(private T $t) {}
  public function set(T $t): void {
    $this->t = $t;
  }
  public function get(): T {
    return $this->t;
  }
}

function evil(dynamic $d1, dynamic $d2): void {
  $x = $d1->get(); // $x: dynamic
  $d1->set($d2);
}

function expect_int(int $i): void {}

function test(): void {
  $b = new Box(4);
  $an_int = $b->get(); // $an_int: int = 3
  evil($b, "hello");
  $not_an_int = $b->get(); // $not_an_int: int = "hello" (!!!)

  expect_int($not_an_int); // TypeHintViolationException
}
```

We allow the `Box<int>` to coerce to `dynamic`, but this is unsafe because the
function writes a `string` into the boxed value without our knowledge. This
means that the `$not_an_int` variable has the wrong type, and we get an
exception in a program that has no Hack errors.

With a sound dynamic type, we can fully eliminate error suppression in Hack,
have trustworthy types, and perform global analysis to constrain the level of
dynamism in the codebase. As it has global effect, which makes per-file
experimental gating impractical, the sound dynamic feature is controlled by
`.hhconfig` flag `enable_sound_dynamic_type`.

## Proposal

We propose the following changes to the `dynamic` type to make it so that any
value typed `dynamic` safely supports dynamic operations.

First, we eliminate coercion entirely, requiring operations with `dynamic` to
use subtyping instead. Next, we define a covariant type constructor `~`
(pronounced "like") such that `~T` represents the union of `dynamic` and `T`. To
describe types that support dynamic, we define
```
newtype supportdyn<+T> as T = (T & dynamic);
```
and introduce a new subtyping relation called *dynamic-aware subtyping* (written
`<D:`), which adds the following simple rule:
```
-----------------------------
supportdyn<mixed> <D: dynamic
```
We also introduce the `upcast` expression to allow us to take values into
`dynamic`. Its typing rule is
```
e : U    U <D: T
----------------
 e upcast T : T
```
Finally, we define normal subtyping (written `<:`) rules for `supportdyn<T>`.

### Like types

These are the simplest part of the proposal. We use `~T` to signal that a value
may have type `T`, but it may also be a dynamic value. Due to standard union
rules, a `~T` only supports operations that are valid for `T` (`dynamic` admits
any operation). For example, it is a type error to call a method on `~string`,
even if the call would succeed at runtime.

Many PHP builtins counterintuitively return `false` in exceptional cases, and a
value otherwise, e.g. `file_get_contents`. If we were to type the function
return as `(string | bool)`, every use site must now statically check this
union, even if the overwhelming majority of callsites to not observe `false`.
A less invasive approach is to give the function a type `~string`. Users are now
aware that they may not be holding a `string`, but they have a choice in whether
to check. The rest of the proposal describes how like types are used to remove
lying types like the one observed in the introduction.

Like types are transparent to the runtime, enforced as their inner type. A
function that expects a `~string` will throw if you pass it a `float`. We have
also considered a variant where like types are not enforced -- none of the
semantics of `dynamic` require enforcement. However, we expect to mainly be
weakening ill-typed code rather than adding types to untyped code, so this
approach preserves existing enforcement behavior.

### Upcast

Hack has an unsafe cast feature that can cast a value to any type. Its purpose
is to aid in the elimination of error-suppression comments in Hack:
```
enum E: int { A = 1; }
function f(int $i): E {
  /* HH_FIXME[4110] trust me */
  return $i;
}

==>

function f(int $i): E {
  return HH\FIXME\UNSAFE_CAST<int, E>($i);
}
```

With sound dynamic, we can replace `UNSAFE_CAST<T1, T2>(e)` with `e upcast ~T2`.
Dynamic-aware subtyping is defined naturally for unions, so this is equivalent
to asking if `int <D: dynamic` (more on that in the following section). To
complete the example, we must weaken `f`'s return type.

```
function f(int $i): ~E {
  return $i upcast ~E;
}
```
A consequence of this is we constrain any value that had flowed into an unsafe
cast to support dynamic. Further, it is not legal to unsafe cast arbitrary
`mixed` values, because `mixed` is not a dynamic-aware subtype of `dynamic`.
Instead, the value's type may be at most `supportdyn<mixed>`.

```
function f(mixed $m1, supportdyn<mixed> $m2): void {
    $m1 upcast ~int; // error
    $m2 upcast ~int; // ok
}
```
Also, since we eliminated coercion, technically all non-dynamic values must be
upcast to dynamic in order to flow into dynamic positions. However, for
ergonomics, we allow implicit upcasting when the target type is exactly
`dynamic`
```
function f(): dynamic {
    return 4; // ok
}

function g(): ~string {
    return 4; // not ok
    return 4 upcast dynamic; // ok
    return 4 upcast ~string; // also ok
}
```
The same applies to parameters and properties. Upcast expressions are erased at
compile time.

### <<__SupportDynamicType>>

The type `supportdyn<T>` means: the set of all values that are belong to `T` and
support dynamic operations. Alternatively, this can be read as the
*intersection* of the sets of values represented by `T` and `dynamic`.
Naturally, this means that
```
----------------------------
dynamic <: supportdyn<mixed>
```

> Note: `supportdyn<mixed>` and `dynamic` represent **the exact same set of
values!**

Unlike dynamic values, however, a value with type `supportdyn<mixed>` does *not*
support arbitrary operations, just as the `mixed` type does not.

What does it mean for a value to support dynamic? Above, we showed an example of
calling a method with an arbitrary `dynamic` value. We may also use the returned
value of any operation that succeeds as dynamic:
```
function f(dynamic $d): void {
    $d1 = $d->someMethod();
    $d2 = $d->someProp;
    $d3 = $d[0];
    // etc
}
```

The main idea here is to prevent users from getting a `dynamic` reference to
anything that does not protect its invariants. Primitives like numbers,
`string`, `null`, and booleans all support dynamic trivially. Their values are
immutable and their runtime properties cannot be changed by dynamic operations.
Tuples and closed shapes support dynamic if their components do. *Open* shapes
do not support dynamic, as we cannot be confident the unspecified fields contain
values that support dynamic. To upcast an open shape to dynamic, it must be
wrapped as `supportdyn<shape(...)>`, which requires that the remaining fields
support dynamic. `mixed` and `nonnull` similarly do not support dynamic.

### Functions

Consider an arbitrary function `f`:

```
// current code
function f(int $i): string { <body> }

function evil(dynamic $d1, dynamic $d2): dynamic {
    $x = $d1($d2); // $x: dynamic
    return $x;
}

function test(dynamic $d): void {
    $y = evil(f<>, $d); // $y: dynamic
}
```
Forget passing a dynamic value to `f`, we can get a handle to the whole function
as dynamic and do as we please! Therefore, to ensure `f` can be safely used in a
dynamic context, we must require that it is still type-safe when all of its
parameters are dynamic, and that its return value may be used dynamically. We
introduce a new attribute `<<__SupportDynamicType>>`:
```
<<__SupportDynamicType>>
function ff(T1 $t1): T2 { <body> }
```
that works by checking an overloaded definition of `ff` with a dynamic
signature:
```
function ff(T1 $t1): T2 {
    <body> // typechecks
}
function ff(dynamic $t1): dynamic {
    <body> // must also typecheck
}
```
The type of `ff` is `supportdyn<(function (int): string)>`, or alternatively
`(function (int): string) & (function(dynamic): dynamic)`. That is, if you call
`ff` with an `int`, you will get back a `string`. If you call `ff` with
`dynamic`, you get back `dynamic`. Finally, if you call `ff` with a `~int`, by
the application of the intersection, you will get back a `~string`. Of course,
if the body of `ff` has an error when we check it with a dynamic signature, then
you get an error that `ff` cannot be marked `<<__SupportDynamicType>>`.

More precisely, a function type `function(T1, ..., Tn): Tr <: supportdyn<mixed>`
if `dynamic <D: T1 & ... & dynamic <D: Tn` and `Tr <D: dynamic`.

### Classes

Recall the previous `Box` example, with the constructor ommitted for clarity:
```
class Box<T> {
  private T $t;
  public function set(T $t): void {
    $this->t = $t;
  }
  public function get(): T {
    return $this->t;
  }
}
```
If a `Box` is to support being used as a dynamic value, then

- all of its methods (incl. inherited) must support dynamic
- any public properties must support dynamic writing and reading
- any class that extends `Box` must also support dynamic

Consider the `set` method:
```
  public function set(T $t): void {
    $this->t = $t;
  }
  public function set(dynamic $t): dynamic {
    $this->t = $t; // type error!
  }
```
In its dynamic version, we are writing a `dynamic` value to the property `t`, so
we will get a type error unless we weaken its type
```
  private ~T $t;
```
and consequently weaken the `get` method
```
  public function get(): ~T {
```

Now let's consider the `get` method:
```
  public function get(): ~T {
    return $this->t;
  }
  public function get(): dynamic {
    return $this->t; // type error!
  }
```
The result of the `get` method must be a valid value that can be typed
`dynamic`, so `get`'s return type `~T` must be be a dynamic-aware subtype of
`dynamic`. We can achieve this by constraining the type parameter
```
class Box<T as supportdyn<mixed>> {
```
and we are finally done.
```
<<__SupportDynamicType>>
class Box<T as supportdyn<mixed>> {
  private ~T $t;
  public function set(T $t): void {
    $this->t = $t;
  }
  public function get(): ~T {
    return $this->t;
  }
}
```
For any valid `Box<T>`, we now have `Box<T> <: supportdyn<Box<T>>` and therefore
`Box<T> <D: dynamic`.

Now let's bring back the original example:
```
function evil(dynamic $d1, dynamic $d2): void { ... }
<<__SupportDynamicType>> function expect_int(int $i): void {}

function test(): void {
  $b = new Box(4);
  $an_int = $b->get(); // $an_int: ~int = 3
  evil(
    $b /* upcast dynamic */,
    "hello" /* upcast dynamic */
  );
  $not_an_int = $b->get(); // $not_an_int: ~int = "hello" (ok)

  expect_int($not_an_int); // TypeHintViolationException
}
```
We are no longer lying to the user that `get` only returns `int`s. The call to
`expect_int` will still throw, but this is the price of dynamism. However, if
the user sees a non-dynamic type `int`, they can now be confident that its
runtime value is actually an integer.

### Pessimisation

The process of weakening and constraining types in a codebase to enable `<<__SupportDynamicType>>` and admit existing use of legacy `dynamic` is called *pessimisation*. Users are free to pessimise their code however they like. For
example, we could have instead pessimised the `Box` example as
```
<<__SupportDynamicType>>
class Box<T> {
  private dynamic $t;
  public function set(dynamic $t): void {
    $this->t = $t;
  }
  public function get(): dynamic {
    return $this->t;
  }
}
```

We also allow enforced types to behave as like types when writing so that we do
not need to weaken them:
```
<<__SupportDynamicType>>
class IntBox {
  private int $t;
  public function set(int $t): void {
    $this->t = $t;
    // we're acting like `t` has type `~int` for the dynamic pass's assignment
  }
  public function get(): int {
    return $this->t;
  }
}
```

## Conditionally-dynamic classes

The restriction of `<T as supportdyn<mixed>>` in our `Box` is particularly
onerous for library classes. For example, it precludes Hack containers from
instantiation with classes that do not support dynamic.
```
<<__Sealed(KeyedContainer::class), __SupportDynamicType>>
interface Container<+Tv as supportdyn<mixed>> extends Traversable<Tv> {
```
The Shack formalization of dynamic allows for arbitrary constraints on classes
to support dynamic provided that the core requirements for reading and writing
hold under those constraints. We propose an instance of this by allowing
```
<<__SupportDynamicType>>
class C<T> {}
```
where `C<T> <D: dynamic` if `T <D: dynamic`.  With this constraint, we can now
write the `Box` example without constraining `T`.
```
class Box<T> {
  private ~T $t;
  public function set(T $t): void {
    $this->t = $t;
  }
  public function get(): ~T {
    return $this->t;
  }
}
```
The method `get` passes the `<<__SupportDynamicType>>` check because we can now
assume `T` was instantiated with a subtype of `dynamic`, so the return
requirement `~T <D: dynamic` holds.

## Inheritance

To aid in the transition of untyped hierarchies that leverage error suppression,
we allow methods that return any `T` to override methods that return dynamic,
provided that `T <D: dynamic`. This is similar to the automatic upcasting to
dynamic shown above.

```
<<__SupportDynamicType>>
class A1 {
  public function f(): ~string { ... }
}
<<__SupportDynamicType>>
class A2 extends A1 {
  public function f(): dynamic { ... }
}
<<__SupportDynamicType>>
class A3 extends A2 {
  public function f(): int { ... }
}
```

even though `int </: dynamic` under regular subtyping. This allowance is not
transitive, and so the following is an error

```
<<__SupportDynamicType>>
class B1 {
  public function f(): ~string { ... }
}
<<__SupportDynamicType>>
class B2 extends B1 {
  public function f(): int { ... } // error
}
```

Note that it would be perfectly sound to allow
```
<<__SupportDynamicType>>
class C1 {
  public function f(): ~string { ... }
}
<<__SupportDynamicType>>
class C2 extends C1 {
  <<__Override>>
  public function f(): ~int { ... }
}
```
This is again because `~string` and `~int` represent the exact same set of
values. In fact, `~T1` is semantically equivalent to `~T2` for all types where
`T1 <D: dynamic` and `T2 <D: dynamic`. However, this is not very ergonomic, so
we propose an alternative
```
<<__SupportDynamicType>>
class D1 {
  public function f(): ~string { ... }
}
<<__SupportDynamicType>>
class D2 extends D1 {
  <<__Override, __DynamicOverride>>
  public function f(): ~int { ... }
}
```

The strictest option is to require regular `<:` subtyping for overrides, in
which case the example above must be weakened to
```
<<__SupportDynamicType>>
class E1 {
  public function f(): ~arraykey { ... }
}
<<__SupportDynamicType>>
class E2 extends E1 {
  public function f(): ~int { ... }
}
<<__SupportDynamicType>>
class E3 extends E2 {
  public function f(): int { ... }
}
```
# Future

## Runtime dynamism

Classes and methods can still be referenced via strings in the runtime, which
creates a blind spot for `<<__SupportDynamicType>>`. In general, it is not safe
to say that a class does not support dynamic if there exist runtime dynamic
references to it. We would like to extend the concept of
`<<__DynamicallyCallable>>` to cover these cases, and tie it to
`<<__SupportDynamicType>>`. This would make it so that all dynamically
referenced functions/classes support dynamic, and those that do not would fatal
on dynamic reference.

## Mocking

Function mocking for tests heavily depends on dynamism. We would like to avoid a
situation where a function is only marked `<<__SupportDynamicType>>` so that it
can be used dynamically in a test, and otherwise has fully static usage. With
this in mind, we are considering some restricted primitives to get `dynamic`
handles on non-dynamic targets for testing.

# Decision points
- General syntax
- Which parts of sound dynamic will we ship?
  - `~T` vs. `HH\FIXME\POISON_MARKER<T>`
  - `supportdyn<T>` vs. `HH\FIXME\SUPPORTDYN_MARKER<T>`
  - `e upcast T` vs. `HH\FIXME\UNSAFE_CAST<_, T>` (like types only)
  - naming of `<<__SupportDynamicType>>`
- Inheritance behavior
  - Proposed behavior
    - Lints?
  - Required `<<__DynamicOverride>>`
  - Only regular subtyping
