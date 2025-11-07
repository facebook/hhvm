## [HIP]  `__Sealed` methods

**First proposal**: 22/4/2024

**Updated**: 16/10/2025

**Author**: Francesco Zappa Nardelli

**Contributors**: Adi Kumar, Catherine Gasnier, Mistral Contrastin, Jake Cordero, Scott Owens, Shashank Kambhampati.

**TL;DR**  Following feedback from programmers it seems that there is interest in having a method-level `__Sealed` attribute that controls which subclasses can override the method.  This document proposes a design, a prototype implementation is available for experimentation.

**Description**:  we propose to add a method-level `__Sealed` attribute.  Semantics is defined below, according to the placement of the attribute.

1. **Sealed method defined in a concrete class.**

If the attribute is added to a method defined in a **concrete class**, the attribute controls which subclasses can override the method.  This part of the design is straightforward and is exemplified by the following examples.  The simplest use-case is below, which would be **accepted** since `foo` is sealed with class `D`, and overridden in class `D`.

```php
class C {
  <<__Sealed(D::class)>>
  public function foo(): void {  echo "I am foo in C\n"; }
}

class D extends C {
  <<__Override>>
  public function foo(): void { echo "I am foo in D\n"; }
}
```

On the other hand, continuing the example,  class E would be **rejected** as it is not allowed to override `foo` in `C`.

```php
...

class E extends C {
 <<__Override>>
  public function foo(): void {  echo "I am foo in E\n"; }
}
```

Class-level `__Sealed` attributes only impose restrictions on direct extends, so a class that extends `D` can override `foo` again.  We mimic the same semantics with method-level `__Sealed` attributes, so building on our running example, the following is accepted:

```php
...

class F extends D {
  <<__Override>>
  public function foo(): void { echo "I am foo in F which extends D\n"; }
}
```

Similarly, it is possible to introduce new `__Sealed` restrictions in subclasses, eg. the following is accepted:

```php
class C {
  <<__Sealed(D::class)>>
  public function foo(): void {  echo "I am foo in C\n"; }
}

class D extends C {
  <<__Override, __Sealed(E::class)>>
  public function foo(): void { echo "I am foo in D\n"; }
}

class E extends D {
  <<__Override>>
  public function foo(): void { echo "I am foo in E\n"; }
}
```

It is arguable whether the following should be accepted or rejected, since there is no analogous scenario in class-level `__Sealed` attributes:

```php
class C {
  <<__Sealed(D::class)>>
  public function foo(): void {  echo "I am foo in C\n"; }
}

class D extends C {}

class E extends D {
  <<__Override>>
  public function foo(): void { echo "I am foo in E\n"; }
}
```

We propose to reject this code so that the `__Sealed(D::class)` attribute in C can be interpreted as _"if foo in C is overridden, then it is overridden in D"_.

2. **Sealed method defined in an abstract class.**

If the attribute is added to an abstract method defined in an **abstract class**, then we have two options (and we should support both):

* the `__Sealed` attribute can specify which **classes** can implement the abstract method, as in the class `D` below:

```php
abstract class C {
  <<__Sealed(D::class)>>
  abstract public function foo(): void;
}

// The __Sealed attribute can specify the class
// that implements the abstract method
class D extends C {
  <<__Override>>
  public function foo(): void { echo "I am foo in D\n"; }
}
```

The abstract class `C` might also defer the overriding implementation of `foo` to a trait, as below.

```php
// method_abstractclass_02.php

abstract class C {
  <<__Sealed(D::class)>>
  abstract public function foo(): void;
}

trait T {
  <<__Override>>
  public function foo(): void { echo "I am foo in T\n"; }
}

class D extends C {
  use T;
}
```

* the `__Sealed` attribute can specify which **traits** can implement the abstract method.  Arbitrary classes can then use the traits, as in the valid code below:

```php
// method_abstractclass_03.php

abstract class C {
  <<__Sealed(T::class)>>
  abstract public function foo(): void;
}

// The __Sealed attribute can specify the trait
// that implements the abstract method.
trait T {
  public function foo(): void { echo "I am foo in T\n"; }
}

// Arbitrary subclasses of C can then use the trait.
class E extends C {
  use T;
}

class F extends C {
  use T;
}
```

If a method is sealed with a trait, then it is not possible to redefine the trait method in the class that uses the trait (otherwise the `__Sealed(T::class)` annotation would be useless).  The following is thus rejected:

```php
// method_abstractclass_04.php

abstract class C {
  <<__Sealed(T::class)>>
  abstract public function foo(): void;
}

trait T {
  <<__Override>>
  public function foo(): void { echo "I am foo in T\n"; }
}

class D extends C {
  use T;
  // This should be rejected
  <<__Override>>
  public function foo(): void { echo "I am foo in D\n"; }
}

class E extends C {
  // This should be rejected
  <<__Override>>
  public function foo(): void { echo "I am foo in E2\n"; }
}

```

If a method is sealed with a trait, then it is not possible to redefine the trait method in the class that uses the trait (otherwise the `__Sealed(T::class)` annotation would be useless).  The following is thus rejected:

```php
// method_abstractclass_05.php

abstract class C {
  <<__Sealed(T1::class)>>
  abstract public function foo(): void;
}

trait T1 {
  <<__Override>>
  public function foo(): void { echo "I am foo in T1\n"; }
}

trait T2 {
  <<__Override>>
  public function foo(): void { echo "I am foo in T2\n"; }
}

class D1 extends C {
  use T1;  // this is ok
}

class D2 extends C {
  use T2;  // this is rejected
}
```

In both cases the rules for transitive visibility of `__Sealed` attributes for methods defined in abstract classes and traits are  similar to those for methods defined in classes (eg. no transitive interpretation).

3. **Sealed method defined in a trait**

If the attribute is added to methods defined in a **trait**, then again, the `__Sealed` attribute can specify which classes or which traits can override the method.  The following is thus accepted:

```php
// method_trait_01.php

trait T1 {
  <<__Sealed(C::class, T2::class)>>
  public function foo(): void { echo "I am foo in T1\n"; }
}

// class C can override foo in T1
class C {
  use T1;

  <<__Override>>
  public function foo(): void { echo "I am foo in C\n"; }
}

// trait T2 can override foo in T1, T2 can then be used by arbitrary classes
trait T2 {
  use T1;

  <<__Override>>
  public function foo(): void { echo "I am foo in T2\n"; }
}

class D {
  use T2;
}
```

while the following overrides are rejected:

```php
// method_trait_02.php

trait T1 {
  <<__Sealed(C::class, T2::class)>>
  public function foo(): void { echo "I am foo in T1\n"; }
}

// class D cannot override foo from T1
class D {
  use T1;

  <<__Override>>
  public function foo(): void { echo "I am foo in D\n"; }
}

// trait T3 cannot override foo from T1
trait T3 {
  use T1;

  <<__Override>>
  public function foo(): void { echo "I am foo in T3\n"; }
}
```

The following is rejected too (the rule of thumb is _"if T1 is inlined in E1 then E2 cannot override foo"_):

```php
// method_trait_03.php

trait T1 {
  <<__Sealed(C::class, T2::class)>>
  public function foo(): void { echo "I am foo in T1\n"; }
}

class E1 {
  use T1;
}

class E2 extends E1 {
  <<__Override>>
  // this is rejected
  public function foo(): void { echo "I am foo in E2\n"; }
}

```

**Syntactic restrictions:**

- Sealed cannot be used on constructors and private methods
- Sealed cannot be used on methods defined in interfaces
- Methods cannot be sealed to names that are not visible (eg. `internal` to a different module)

**Property:**

If a method is sealed with an empty list of classes, then it cannot be overridden (eg. it is final).

**Implementation**:

**Hack**: we have a prototype implementation that can be used for experimentation:
* allow `__Sealed` on methods,
* store in shallow decls
* while doing override checks, enforce the sealed semantics whenever the overridden method has the relevant attribute

**HHVM**: HHVM enforces the class-level Sealed attributes at class/trait load time.  We argue that HHVM does not need to enforce method-level `__Sealed` attributes.  If needed, we can mimic the enforcement for method-level Sealed attributes.  No extra run-time checks / overhead are needed, and no changes to Func objects are required.
