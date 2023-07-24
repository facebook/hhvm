Traits have special behavior in Hack when used with modules.

## Marking traits internal
An internal trait can only be used within the module it's defined. Internal traits can have public, protected, or internal methods.

```hack
//// newmodule.hack
new module foo {}

//// foo.hack
module foo;
internal trait TFoo {
  public function foo(): void {}
  internal function bar(): void {}
}
```

```hack no-extract
module bar;
public class Bar {
  use TFoo; // error, TFoo is internal
}
```

## Public traits

Hack supports two different module semantics for public traits.

By default, since public traits by nature can have their implementations copied to any class in any other module, public traits cannot access any internal symbols or implementations from the module they are defined in. You can think of public traits as not belonging to any specific module, even if they are defined within one.

```hack no-extract
module foo;
internal class Foo {}
// Assume TFoo is defined as before
public trait TBar {
  use TFoo; // error, TBar is a public trait, it cannot access internal symbols in module foo
  public function foo(): mixed {
    return new Foo(); // error, TBar is a public trait, it cannot access internal symbols in module foo
  }
}
```

As experimental feature, the programmer can however declare a public trait to belong to the module where the trait was itself defined, by adding the `<<__ModuleLevelTrait>>` attribute to the trait definition.  By doing so the methods of the trait can access the internal symbols and implementations of the module they are defined in.  For instance the following code is accepted:

```hack no-extract
/// file a.php
<<file: __EnableUnstableFeatures('module_level_traits')>>

module A;

internal function foo(): void { echo "I am foo in A\n"; }

<<__ModuleLevelTrait>>
public trait T {
   public function getFoo(): void {
     foo();             // both getFoo and foo belong to module A
   }
}

/// file b.php
module B;

class C { use T; }

<<__EntryPoint>>
function bar(): void {
  (new C())->getFoo();  // prints "I am foo in A"
}
```

Module level traits are especially useful in conjunction with the `<<__Sealed(...)>>` attribute to export selectively internal module functionalities, mimicking C++ friends modules.


Public traits cannot have internal methods or properties, as they may be used by classes outside of any module.

```hack no-extract
public trait TBar {
  internal function foo(): void {} // error, public traits cannot have internal methods or properties
}
```
