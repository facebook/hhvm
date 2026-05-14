Author: Max Heiber
Status: accepted and implemented

# HIP: `__NeedsConcrete` attribute

## Real-World Example

> This has been modified for open-source audiences. See the [original document](https://fburl.com/gdoc/40aqaxob) for the original code

There are tons of methods you can call in WWW today that will immediately blow up due to weaknesses in `abstract` checking. For example, a call to `AssocBackedEntQuery::query()` straightforwardly fails with "cannot instantiate abstract class". Code (abridged):

```hack
abstract class AssocQuery
  extends Fetcher
  implements QueryHasTime, QueryBackedByAssoc {

  protected function cloneQuery(): this {
        return self::query();
   }
}

trait Fetcher {
  final public static function query(): this {
    return new static();
  }
```

With this proposal, we prevent calling `AssocQuery::query` and `cloneQuery` directly via the `__NeedsConcrete` method attribute: the methods are then safe with no other changes to WWW.

## **Motivation**

Errors from attempting to access abstract methods, constructors, and constants account for one of the largest known categories of runtime errors preventable with better type-checking:

**SEV prevention:** **[redacted](https://fburl.com/hack_needs_concrete)**

**Dev Efficiency:** There are **[redacted](https://fburl.com/hack_needs_concrete)** static methods in WWW that will fail if called ([D71133334](https://fburl.com/diff/t0lzpuii)). Users work around this today through fixing runtime errors and maintaining complicated invariants. **We can shift these errors left.**

\* Both an over and under-estimate: this proposal doesn't make classname or class pointers any safer, that is saved for potential follow-on work un-blocked with this HIP. Under-counting comes from gaps in our ability to attribute.

## **Solution**

## Solution: High-level explanation

**Definitions**:

* **`abstract`-safety**: `hh` is abstract-safe if it prevents attempts at runtime to call a constructor that doesn't exist, call an unimplemented abstract method, or access an unimplemented abstract constant.
* A **concrete class** is a class that is either non-abstract or is final+not-`__ConsistentConstruct`. The intuition behind this definition is "in a concrete class there are no unimplemented members".

**Invariant**: In a `__NeedsConcrete` method, we know that `static` points to a concrete class.

The checks introduced in this HIP all follow from **leveraging** or **preserving**  the invariant to get **more `abstract`- safety**. I suspect with this proposal we'll be abstract-safe modulo dynamism, modulo classname\<T\>/class\<T\>, and existing type holes.

Here's one of the ways we **leverage** the invariant:

- In a `__NeedsConcrete` method, `static::abstract_method()` is allowed

Here's one of the ways we **preserve** the invariant:

- ### If method `Abs::m` is marked `__NeedsConcrete`, and `Abs` is not a concrete class, then `Abs::m()` is not allowed

### Why final, non-\_\_ConsistentConstruct classes are considered concrete

Because there's nothing wrong with code like the following, which is really common in WWW (abridged from [real code](https://fburl.com/code/y1lfyhnq)). There are **[redacted](https://fburl.com/hack_needs_concrete)** cases in WWW ([D71548214](https://fburl.com/diff/wnrgpofg)) where treating final+not-`__ConsistentConstruct` classes as concrete prevents spurious hh errors:

```

abstract class GraphQLClientBaseArtifactData
  implements IGraphQLClientArtifactData {

  private function __construct()[] {}

  public static function addToOutput(
    GraphQLQueryClientContext $context,
    inout GraphQLHackClientSandboxCacheOutput $output,
  )[write_props]: void {
    static::addToOutputImpl($context, inout $output);
  }
}

abstract final class GraphQLClientBloksQueryInputArtifactData
  extends GraphQLClientBaseArtifactData
  implements IGraphQLClientArtifactData {}
```

*See also why \_\_ConsistentConstruct matters, below*

### Solution By Example

> These examples are all adapted from the test suites for [D71745959](https://fburl.com/diff/6fh9z98q) and [D69055023](https://fburl.com/diff/gjw7t67e)
> See the next section for an algorithm sketch and link to full algo

We error when there's an access of an abstract member via `static` and `static` isn't known to point to a concrete class:

```java
<?hh

abstract class A {
  public static function m1(): void {
    // hh error: unsafe because `static` might not refer to a concrete class.
    static::m2();
  }
  public static abstract function m2(): void;
}

```

The easiest fix in such cases is to add `<<__NeedsConcrete>>`:

```java
<?hh

abstract class A {
  public static function m1(): void {
    // hh error: unsafe because `static` might not refer to a concrete class.
    self::m2();
  }

  <<__NeedsConcrete>>
  public static function m2(): void {
    static::m3(); // ok: the attribute ensures `static` refers to a concrete class
  }

  public static abstract function m3(): void;
}

```

The following shows how we check calls to \_\_NeedsConcrete methods where the receiver is a class ID. Nice and simple, the receiver must be concrete:

```java
<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function m1(): void {
    static::m2(); // ok: the attribute ensures `static` refers to a concrete class
  }

  public static abstract function m2(): void;
}

<<__EntryPoint>>
function main(): void {
  A::m1(); // hh error and runtime error
}

```

Note that the runtime behavior is for `parent` to forward a value for `static`. The following example shows both what happens at runtime and my first stab at an error message:

*I admit that the error message is hard to understand, but it's merely making explicit what was previously hidden complexity*

```java
<?hh

abstract class C1 {
  <<__NeedsConcrete>>
  public static function m2(): void {
    static::m3();
  }
  public static abstract function m3(): void;

}

abstract class C2 extends C1 {
  public static function m1(): void {
    // Cannot call `C1::m2` (a `<<__NeedsConcrete>>` method) via `parent`. It requires `static` to refer to a concrete class, but `parent` sets `static` to a class that may not be concrete. (Typing[4490])
    parent::m2();
  }
}

class C3 extends C2 {
  public static function m3(): void {
    echo "m3\n";
  }
}

<<__EntryPoint>>
function main(): void {
  C3::m1(); // prints "m3"
  // C2::m1(); // fatal: "cannot call abstract method C1::m3()"
}

```

No errors here:

```java
<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function m1(): void {
    static::m2(); // ok
  }
  <<__NeedsConcrete>>
  public static function m2(): void {}
}

```

`abstract final` classes that are non-\_\_ConsistentConstruct are common in WWW. We treat such cases as concrete, so there are no errors here:

```java
<?hh

abstract class C1 {
  abstract const int X;
  <<__NeedsConcrete>>
  public static function nc(): void {}

  public static abstract function abs(): void;

}

// C2 is concrete because it is final+non-__ConsistentConstruct
abstract final class C2 extends C1 {
  const int X = 3;
  public static function m(): void {
    // all the following are ok because C2 is concrete
    static::nc();
    static::abs();
    static::abs<>;
    static::X;
  }

  public static function abs(): void {}
}

```

Here's why \_\_ConsistentConstruct matters. C2::m() below would fail at runtime, since it would try to instantiate an abstract class:

```java
<?hh

<<__ConsistentConstruct>>
abstract class C1 {
  abstract const int X;
  <<__NeedsConcrete>>
  public static function nc(): void {
    new static();
  }

  public static abstract function abs(): void;

}

// C2 is *not* concrete even though it is `abstract final`.
// C2 has an unimplemented member (the constructor)
abstract final class C2 extends C1 {
  const int X = 3;
  public static function m(): void {
    static::nc(); // runtime error - because it calls `new` and the constructor for C2 doesn't exist
  }

  public static function abs(): void {}
}

```

As always, if a method is \_\_NeedsConcrete then `static` refers to a concrete class:

```java
<?hh


<<__ConsistentConstruct>>
abstract class A {
  abstract const int X;
  public static abstract function abs(): void;

  <<__NeedsConcrete>>
  public static function ok(): void {
    static::X; // ok
    static::abs<>; // ok
    new static(); // ok
  }
}

```

Hierarchy checks, similar to the rule that an abstract method cannot override a non-abstract method:

```java
<?hh

abstract class A {
  public static function nc(): void {}
  public abstract static function abs(): void;
}

abstract class B extends A {
// hh error: Cannot declare this method as __NeedsConcrete, since it overrides a non-__NeedsConcrete method (Typing[4487])
  <<__NeedsConcrete>>
  public static function nc(): void {
    static::abs(); // HHVM error
  }
  public static abstract function abs(): void;
}

function example(classname<A> $klass): void {
  $klass::nc();
}

<<__EntryPoint>>
function main(): void {
  example(B::class);
}

```

## Implementation

Here's a sketch of how we check `static::foo() or MyClass::foo():`

```

let containing_class_is_final_and_concrete cls =
      final class && (non_abstract cls || non_consistent_construct cls)

let static_points_to_concrete_class env =
      containing_method_has_needs_concrete_attribute
      || not env.in_static_method
      || containing_class_is_final_and_concrete env.containing_class

let check_static_call receiver env = match receiver with
 | CIstatic when not (static_points_to_concrete_class env) ->
          if callee_has_needs_concrete_attribute || callee_is_abstract then
           error ()
  | CI class_name when callee_has_needs_concrete_attribute && (not class_is_concrete class_name) -> error ()
  | _ -> ....
```

The [implementation (D71745959)](https://fburl.com/diff/6fh9z98q) checks accesses of constructors, constants, and methods via super, self, parent, and class ID, but the rules are obvious once it's clear what the invariants are.

##  Migration Plan

* Codemod to add the `__NeedsConcrete` attribute to **[redacted](https://fburl.com/hack_needs_concrete)** methods in WWW:
  * Fixpoint until nothing changes:
    * If a static method `m` of a non-concrete class that is not \_\_NeedsConcrete accesses an abstract or \_\_NeedsConcrete method via static/self/parent, add the \_\_NeedsConcrete attribute to `m`
    * If `NeedsConcrete` method `C2::m` overrides a non-`\_\_NeedsConcrete` method `C1::m`, add the `__NeedsConcrete` attribute to `C1::m`
  * Note: the logic will be very similar to that implemented in the analysis: [D71133334](https://fburl.com/diff/t0lzpuii)
* fixme or fix the small number of new errors in WWW

## Trade-offs and Stats about WWW

Benefits:

* Safer code
* Easier to for users to write+maintain code
  * My understanding is that today's WWW is the result of **great effort** to make code work well as much as possible
    * complex invariants around hierarchies and concreteness
    * fixing issues discovered in production
    * fixing issues discovered in testing
  * We reduce future WWW development and maintenance cost through shifting left
* Un-block further Safe Abstract work for more safety

Cost:

* `__NeedsConcrete` needs to be learned by users
  * My take: it's better to have Hack track concreteness for you than to manage complex invariants around `self` and `static` without tool assistance
* Implementation complexity: not much, fairly orthogonal [D71745959](https://fburl.com/diff/6fh9z98q)
* Changes to existing code:
  * Add \_\_NeedsConcrete attribute in **[redacted](https://fburl.com/hack_needs_concrete)** places ([source](https://fburl.com/diff/t0lzpuii)). Note that this is easily reversible (just delete the attribute!)
  * introduce **[redacted](https://fburl.com/hack_needs_concrete)** new errors at call sites in WWW ([source](https://fburl.com/diff/wnrgpofg)). Even if we're off by an order of magnitude that's still not many fixmes/refactors.

## Appendix

### Potential Follow-on Work: class pointers and class names

`class<T>` and `classname<T>` are still not abstract-safe, but may be addressed in a future proposal.

We break the problem into pieces

* This proposal *only* adds `<<__NeedsConcrete>>`
* No new checking for `class<T>` or `classname<T>` (yet)

Why:

* Easier migration than moving WWW to full Safe Abstract checking at once
* Incremental value and learnings
* Ordering: `__NeedsConcrete` would be needed for `class<T>`/`classname<T>` checking anyway

### Potential Follow-on Work: no longer need to pass value for `static` through `self` and `parent`

Python has inheritance for static methods, similar to Hack, but more clearly separates which classes get to access the current runtime method.:

```py
class C:
    @classmethod
    def m1(cls):
        # `cls` points to the current class
        # like `static` in Hack static methods or `this` in Flow static methods
        cls.m2()
        ...

    @staticmethod
    def m2():
        # No access to "current runtime class",
        # is more like a Java static method
        ...
```

> Jan Oravec and Vassil Mladenov have iirc mentioned wanting to get Hack closer to Python's explicitness and clarity w.r.t. late static bindings. And Mistral Contrastin has pointed out that explicitly passing the class at least provides a helpful intuition for how PHP/Hack Late Static bindings work.

### Why not fine-grained Method Requirements

A source of incompleteness is:

- Method C::`lsb` is \_\_NeedsConcrete
- Child1 inherits `lsb` and does not override it
- All the abstract things accessed in `lsb` are concrete in `Child1`
- So we have a method (`Child1::lsb`) which is `__NeedsConcrete` but doesn't need to be:

```
<hh
abstract class C {
    public abstract static function m(): void;
    // C::lsb is the common ancestor of weird hierarchy members Child1::lsb and Child2::lsb
    // The --needs-concrete analysis will show that C::lsb should be <<__NeedsConcrete>>
    // because ::lsb() will fail if the LHS is an abstract class that does not have an implementation of `m`
    public static function lsb(): void {
        static::m();
    }
}

abstract class Child1 extends C {
    public static function m(): void {}
    // (unwritten) member Child1::lsb is a weird_hierarchy_member
    // because it is <<__NeedsConcrete>> but it doesn't actually need a concrete receiver
}

abstract class Child2 extends C {
    public static function m(): void {}
    // (unwritten) member Child2::lsb is a weird_hierarchy_member
    // because it is <<__NeedsConcrete>> but it doesn't actually need a concrete receiver
}


function example(): void {
    // incompleteness. If our checking were more precise, we would know that
    // Child1::lsb() is safe at runtime. However, Safe_abstract_check will
    // produce an error here (assuming C::lsb is marked as <<__NeedsConcrete>>)
    Child1::lsb();
}
```

We could address the incompleteness by using a more fine-grained notion than just concrete/non-concrete.

However, the incompleteness does not seem so bad in practice. The analysis in [D71548214](https://fburl.com/diff/wnrgpofg) found **[redacted](https://fburl.com/hack_needs_concrete)** such cases of this in WWW. Even if we're off by an order of magnitude, that's still not many.

### Prior Art from Other Languages

# **Prior Art from Other Languages for type-checking Late Static Bindings**

Supplement to [Proposal: Shift Left `static::` and `classname` errors](https://fburl.com/gdoc/2yjhdsle)

## Python/Pyright:

* Has late static binding (static methods that are inherited where `cls` refers to the runtime rather than lexical class)
* Has a similar soundness hole to Hack's ([GitHub issue I opened](https://github.com/microsoft/pyright/issues/8550))

## Python/Pyre:

* Has late static binding (static methods that are inherited where `cls` refers to the runtime rather than lexical class)
* Doesn't prevent even direct calls to abstract static methods ([example](https://fburl.com/workplace/cvv83f56)), making it easy to get runtime errors.

## TypeScript:

* Has late static binding: `this` in a static method refers to the runtime class, and static methods are inherited.
* Abstract and interface methods cannot be static, so similar problems to Hack's do not arise.

## Flow

* No abstract classes or static interface methods, so similar problems to Hack's do not arise.

## Java, Scala, Kotlin, and C\#:

* Abstract and interface methods cannot be static, so similar problems to Hack do not arise.
* Notes:
  * I tested with C\# 7, newer versions may be different
  * Java afaict does not have LSB: it doesn't seem possible to get a handle to the runtime class as distinct from the lexical class. Not sure about the other languages (but moot point since there's no abstract static)
  * Scala and Kotlin technically do not have static methods, and instead have "companion objects". Afaict, companion objects have no abstract members.
  * Banning abstract static methods seems to be the respectable thing to do, but imo the ship has sailed for WWW. tbgs found **[redacted](https://fburl.com/hack_needs_concrete)** uses of 'abstract.+static' and 'static.+abstract' and that doesn't even cover occurrences in interfaces.


### Prior Art for Hack

The now-partially-outdated-but-perhaps-informative proposal [Proposal: Safe Abstract (formerly known as Shift Left Abstract Static)](https://fburl.com/gdoc/oqpo8udj). That proposal also addresses `classname<T>` and `class<T>` and has [formalization in Alloy and Lean](https://github.com/mheiber/lsbsteak/)

See also the Prior Art section of [Proposal: Safe Abstract (formerly known as Shift Left Abstract Static)](https://fburl.com/gdoc/pjlp4q5c).
