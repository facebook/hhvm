# HIP: Implicit pessimisation

## Motivation

The existence of HH_FIXME and use of unsound dynamic in a large Hack codebase
precludes us from statically determining which values are used dynamically, and
which classes and functions may safely opt out. We would like to start from a
position that every value in the codebase supports safe dynamic operations. This
makes it safe for a value to fall into the blast radius of an HH_FIXME, because
it may simply be replaced with an upcast to an appropriate like type instead
using `HH\FIXME\UNSAFE_CAST`, and flow unimpeded into functions that support
dynamic calls.

# Proposal

We propose a uniform pessimisation strategy where every class and function in
the codebase is implicitly marked `<<__SupportDynamicType>>`. To meet the
conditions for supporting dynamic, we implicitly interpret as follows:

All function and method returns of unenforced types become like types.
```
type D<T> = T;
class C {
  public function good(): int {
    return 4;
  }
  public function bad(): D<int> {
    /* HH_FIXME[4110] */
    return "hello";
  }
}

function test(): void {
  $c = new C();
  $a = $c->good(); // $a: int
  $b = $c->bad(); // $b: ~int
}
```
Enums get partial enforcement at their base types to prevent override errors.
```
enum Transparent: int as int { A = 1; }
enum Opaque: int { B = 2; }
class C {
  public function f(): int {
    return 4;
  }
  public function g(): arraykey {
    return 4;
  }
}
class D extends C {
  public function f(): Transparent /* ~Transparent & int */ {
    return Transparent::A;
  }
  public function g(): Opaque /* ~Opaque & arraykey */ {
    return Opaque::B;
  }
}
```
Add supportdyn bounds on abstract type constants
```
abstract class A {
  abstract const type T /* as supportdyn<mixed> */;
  public function f(): this::T { ... };
}
```

Interpret `HH\FIXME\UNSAFE_CAST<T1, T2>(e)` as `e upcast ~T1`.
```
  public function bad(): this::T /* like type */ {
    return HH\FIXME\UNSAFE_CAST<string, int>("hello"); // returning ~int
  }
```

Interpret `mixed`, `nonnull`, function types, and open shape types as wrapped
in `supportdyn`.
```
function d(dynamic $d): void {}
function m(mixed $m): void {
  // $m: supportdyn<mixed>
  d($m); // implicit upcast to dynamic
}
```

Weaken returns and inouts in function type hints
```
function f((function(): string) $f): void {
  // $f: supportdyn<(function(): ~string)>
}

function g((function(inout string): void) $g): void {
  // $f: supportdyn<(function(inout ~string): ~void)>
}
```
> The `~void` is unfortunate, but is addressed by the `return await f();`
proposal.

Interpret interface and abstract method returns as like types, as they have no
function body to trigger enforcement.
```
interface I {
  public function f(): int;
}
abstract class A {
  public function g(): int;
}
class C extends A implements I {
  const type T = int;
  public function f(): this::T { ... } // ~this::T overrides ~int
  public function g(): this::T { ... }
}
```
TODO: @sowens for the complex extends / implements case.

## Hierarchy Poisoning

Interpreting unenforced types as like types causes problems in hierarchies.
```
class A {
  public function f(): int { ... }
}
class B {
  const type T = int;
  public function f(): this::T { ... } /* override error */
}
```
We resolve this by manually weakening the type of the parent class ahead of time
to prepare the codebase for implicit pessimisation
```
class A {
  public function f(): ~int { ... }
}
```
We introduce a handful of marker types to aid this preparation
| Marker name            | Current interpretation | Sound dynamic interpretation |
| ---------------------- | ---------------------- | ---------------------------- |
| `POISON_MARKER<T>`     | `T`                    | `~T`                         |
| `TANY_MARKER<T>`       | `_`                    | `T`                          |
| `SUPPORTDYN_MARKER<T>` | `T`                    | `supportdyn<T>`              |

# Future

## <<__NoAutoDynamic>>

Implicit pessimisation locks all classes and functions into supporting dynamic.
In order to migrate away from dynamism, we propose an opt-out mechanism `<<__NoAutoDynamic>>` that can be used for functions that are not dynamically
callable, and classes in the future once they are similarly verified. The
attribute turns off implicit pessimisation for the attached definition. The
definitions
```
class C {}
<<__NoAutoDynamic, __SupportDynamicType>>
class D extends C {}
```
under implicit pessimisation are equivalent to
```
<<__SupportDynamicType>>
class C {}
<<__SupportDynamicType>>
class D extends C {}
```
without implicit pessimisation.

## Elimination of implicit pessimisation

The core goal of this project is to make explicit the use of dynamism in the
codebase by eliminating misfeatures such as HH_FIXME. Once a sufficient number
of classes are marked up with `<<__NoAutoDynamic>>`, we would like to "flip the
switch" and turn off implicit pessimisation. This will require explicit
insertion of `<<__SupportDynamicType>>` and like type hints. Then, any new
classes and functions will be outside dynamic by default, and users can reap the
benefits of static typing.

# Decision points
- (copy paste) Which parts of sound dynamic will we ship?
  - `~T` vs. `HH\FIXME\POISON_MARKER<T>`
  - `supportdyn<T>` vs. `HH\FIXME\SUPPORTDYN_MARKER<T>`
  - `e upcast T` vs. `HH\FIXME\UNSAFE_CAST<_, T>` (like types only)
  - naming of `<<__SupportDynamicType>>`
- Hierarchy overrides in the context of implicit pessimisation
