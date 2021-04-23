Status: This is not a full HIP following the usual template/process, but shared as a HIP for external visibility.

# Converging constants in Hack and HHVM

I’d like to ease a few HHVM restrictions on type constant overriding, and along the way to consolidate the rules around constant inheritance in general.

## Motivation

The attribute-based approach to conditional purity  (and conditional reactivity) worked like this.

```Hack
abstract class A {
  <<__Pure, __OnlyRxIfImpl(I::class)>>
  abstract public function f(): void;
}

interface I {
  <<__Pure>>
  public function f(): void;
}

class C extends A implements I {
  <<__Pure>>
  public function f(): void {}
}

class D extends A {
  // impure
  public function f(): void {}
}
```

The `__OnlyRxIfImpl` attribute says that a method invocation on a value of type A is pure only if that value is also an instance of interface I. Under coeffects, we express this conditional purity with an abstract context constant.

```Hack
abstract class A {
  abstract const ctx C = [defaults];
  abstract public function f()[this::C]: void;
}

interface I {
  const ctx C = [];
}

class C extends A implements I {
  public function f()[]: void {}
}

class D extends A {
  public function f(): void {}
}
```

HHVM doesn’t allow non-abstract constants in an interface to conflict with non-abstract constants in the parent class, and since abstract type constants with defaults are simple non-abstract constants at runtime, we can’t use the current HHVM constant semantics to express the definition of class C.

## Background

*Note: I’ve compacted class definitions as I think the examples are easier to read this way.*

Constants in HHVM can be overridden in subclasses, which is a behavior inherited from PHP.

```Hack
class A           { const int X = 3;              }
class B extends A { const int X = 4; /* allowed*/ }
```

Hack allows this behavior to match the runtime. However, if an *interface* declares a constant with the same name, then we have an error when the subclass is loaded.

```Hack
class A     { const int X = 3; }
interface I { const int X = 4; }

class B extends A implements I {} // runtime error
```

If X were instead an abstract constant, then class B can be defined.

```Hack
abstract class A { abstract const int X; }
interface I      { const int X = 4;      }

class B extends A implements I {} // ok
```

Of course, if the interface weren’t present, class B would have to define the constant itself to satisfy the requirements of A.

## **Type Constants**

The type constants feature of Hack, which provides a form of path-dependent types via where constraints, was built on top of class constants. A type constant can be thought of as a class constant whose value is a type structure. As such, the overriding rules for type constants in the runtime are the same — the following code runs without error.

```Hack
class A           { const type T = int;    }
class B extends A { const type T = string; } // runtime ok, Hack error
```

For type safety, Hack bans this overriding behavior. We’ve reached our first major point of divergence between Hack and HHVM.

There is an escape hatch in Hack, however, in the form of partially abstract type constants. These type constants have an `as` constraint and their values can be overridden in a subclass. At runtime, the `as` constraint is simply erased and the constant is interpreted as a regular, non-abstract type constant. In other words, the following example is identical to the previous example from the runtime’s perspective.

```Hack
class A           { const type T as arraykey = arraykey; }
   // runtime sees `const type T = arraykey;`

class B extends A { const type T = int; } // no Hack error, runtime remains ok
```

This is convenient to developers, who use the feature to define a value for a type constant in a superclass and only override it in a few subclasses, but it adds complexity because it requires Hack to make judgements about the type `this::T` based on the exactness of types.

I added the ability to define default values for abstract type constants.

```Hack
abstract class A  { abstract const type T = arraykey; }
   // runtime sees `const type T = arraykey;`

class B extends A { const type T = int; }
```

At runtime, this uses the same machinery as partially abstract type constants, which allows for drop in replacements without risking behavior changes. In the typechecker, abstract type constants are generally much better behaved than partially abstract type constants. They cannot appear in concrete classes, ban direct references via `self::` and `A::`, and cannot be read with `type_structure`. Without partially abstract type constants, `this::T` can be unambiguously resolved in concrete classes.

Unfortunately, the combination of all the implementation details discussed above thereby prevents the following convenient use case.

```Hack
abstract class A { abstract const type T = arraykey; }
interface I      { const type T = int; }

class B extends A implements I {} // runtime error
```

This is the mirror of the context constant example discussed in the motivation section.

Finally, [traits now support constants](https://hhvm.com/blog/2021/02/16/hhvm-4.97.html). Their previous ban of constants could be trivially circumvented by having the trait implement an interface. However, if a constant conflict comes from an interface via a trait, the conflict is silently dropped. With this in mind, the prior interface conflict example can be side-stepped using a trait.

```Hack
class A     { const int X = 3; }
interface I { const int X = 4; }
trait T implements I {}

class B extends A { use T; } // no runtime error!
```

## Proposal

* We use the abstract keyword instead of the presence of a value to denote abstractness for type constants in HHVM. We have committed changes to expose this in HackC and HHVM.
* We require that any classlike (class, interface, trait) has *at most* a single, canonical concrete constant for a given name. If there is a conflict in two defaults for an abstract type constant, we require the developer to redeclare the desired one. Finally, if there are two conflicting defaults but also a single concrete type constant in the hierarchy, the concrete one wins without error.
    * Trivial, existing behavior
      ```Hack
      abstract class A  { abstract const type T = int; }

      class C extends A {} /* const type T = int; */
      class D extends A { const type T = string; }

      class X extends C {
        const type T = string; // error, cannot override concrete C::T
      }
      ```
    * Inherited concrete winning over a inherited abstract
      ```Hack
      abstract class A   { abstract const type T = int; }
      interface I        { const type T = string; }

      class C extends A implements I {} // ok, C::T = I::T
      class D extends A implements I {
        const type T = int; // error, conflicts with I::T
      }
      ```
    * Two conflicting defaults
      ```Hack
      abstract class A   { abstract const type T = int; }
      interface IAbs     { abstract const type T = string; }

      class C extends A implements IAbs {
        // error, must declare a concrete T
      }
      abstract class D extends A implements IAbs {
        // error, must redeclare T and choose a default
        // (can do abstract or concrete)
      }
      ```
    * Two conflicting defaults, but concrete wins
      ```Hack
      abstract class A   { abstract const type T = int; }
      interface IAbs     { abstract const type T = string; }
      interface IConc    { const type T = float; }

      class C extends A implements IAbs, IConc {
         // ok, C::T = IConc
      }
      ```
    * **Design question**: Two conflicting defaults **with same default**
      ```Hack
      abstract class A   { abstract const type T = int; }
      interface IAbs     { abstract const type T = int; }

      class C extends A implements IAbs {
         // should this error?
      }
      ```
    * Abstract override concrete (from class)
      ```Hack
      abstract class A { const type T = int; }

      abstract class B extends A {
        // error, abstract cannot override concrete
        abstract const type T = string;
      };

      interface I { // same error
        require extends A;
        abstract const type T = bool;
      }

      trait T { // same error
        require extends A;
        abstract const type T = float;
      }
      ```
    * Abstract override concrete (from interface)
        * Note: the runtime doesn’t check `require extends` or `require implements`, so those local errors would be in Hack only. The runtime would error later when the interface is implemented or the trait is used.

      ```Hack
      interface I { const type T = int; }

      abstract class A implements I {
        // error, abstract cannot override concrete
        abstract const type T = string;
      };

      interface I1 extends I { // same error
        abstract const type T = bool;
      }

      trait T1 implements I { // same error
        abstract const type T = float;
      }

      trait T2 { // same error
        require implements I;
        abstract const type T = float;
      }
      ```
    * Abstract override concrete (from trait)
      ```Hack
      trait T { const type T = int; }

      abstract class A {
        // error, abstract cannot override concrete
        use T;
        abstract const type T = string;
      };

      trait U { // same error
        use T;
        abstract const type T = float;
      }
      ```
    * Two conflicting concretes
      ```Hack
      interface I { const type T = int; }
      interface J { const type T = string; }

      interface K extends I {
        const type T = string; // error, conflict with I::T;
      }
      interface L extends I, J {} // error, J::T conflicts with I::T
      ```
    * Inherited concrete winning over inherited abstract
      ```Hack
      interface I { abstract const type T = int; }
      trait Tr    { const type T = string; }

      class C implements I { use Tr; } // C::T = string
      ```

      ```Hack
      interface I { const type T = int; }
      trait Tr    { abstract const type T = string; }

      class C implements I { use Tr; } // C::T = int
      ```
      Note that while we are requiring users to redeclare in cases of conflicting defaults, the bounds in the redeclared type constant must still satisfy the all of the bounds in all of the parents. All of the examples above are `as mixed super nothing`. Example:
      ```Hack
      abstract class A   { abstract const type T as arraykey = arraykey; }
      interface IAbs     { abstract const type T as int; }
      interface IConc    { const type T = float; }

      class C extends A implements IConc {
        // error, IConc::T = float, but T is `as arraykey`
      }

      abstract class D extends A implements IAbs {
        // error, IAbs::T brings in constraint `as int`
        // must redeclare T with a default that satisfies this constraint
      }
      ```
    * The first draft of this proposal did not require users to resolve default conflicts, instead taking the last default in the linearization. Feedback was to require explicit resolution and drop the inherited conflicting defaults.

* We make type_structure start failing on abstract type constants with defaults, to push them to behave closer to abstract type constants without defaults. We also do the same for direct references via the class. Of course, as with all runtime changes, we start by logging and sampling before moving to hard enforcement.
  ```Hack
  abstract class A  {
      abstract const type T;
      abstract const type U = int;
  }

  type_structure("A", "T"); // this fatals
  type_structure("A", "U"); // this currently does not

  function f(
    A::T $a, // this fatals
    A::U $b, // this currently does not
  ): void {}
  ```
* Optional: We ban overriding of class constants in Hack to match the type constant behavior. A benefit of this is that `static::X` and `self::X` will have identical meanings in concrete classes. We then ban overriding of concrete class constants and concrete type constants in HHVM.

  ```Hack
  class A           { const int X = 3;           }
  class B extends A { const int X = 4; /* ban */ }
  ```
* Possibly for coherence, we should add default values to abstract class constants as well.

  ```Hack
  abstract class A  { abstract const int X = 3; }
  class B extends A {}
  class C extends A { const int X = 4; /* override */ }
  ```
    * If we implement this and the previous bullet, the semantics of class constants will become identical to type constants across HHVM and Hack.
