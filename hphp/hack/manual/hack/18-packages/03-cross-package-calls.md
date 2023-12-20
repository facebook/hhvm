Symbols across packages are not directly visible to each other, since they may be in different builds. Accessing a symbol that belongs in a module that is in a different package will generally result in an error. Consider the following PACKAGES.toml:

```toml
[packages]

[packages.production]
uses=["prod.*", "my_prod"] # Package contains all modules that start with `prod`, and the module "my_prod".

[packages.test]
uses=["test.*"]
includes=["production"] # Package depends on the production package

[deployments]
[deployments.production]
packages=["production"]

[deployments.test]
packages=["test", "production"] # Since the test package includes production, they must be deployed together.
```

```hack file:example1.hack
//// test/__MODULE__.hack
new module test.foo {
}
```
```hack file:example1.hack
//// prod/__MODULE__.hack
new module prod.bar {
}
```

```hack file:example1.hack
//// test.hack
module test.foo;
class Foo {
}
```

```hack file:example1.hack
//// prod/prod.hack
module prod.bar;
class Bar {}
<<__EntryPoint>>
public function bad_call(): void {
   // $x = new FooTest();
   // typechecker error: FooTest is in module test.foo
   // which is not visible to package fb-www
}
```
At runtime, the call will succeed only if the active deployment in HHVM is set to a deployment that contains the `test` package.

A package that inclues another package can freely access all of its symbols. For example, in our original PACKAGES.toml, since the tests package includes prod, we can call production code from the test package:

```hack file:example1.hack
//// test_bar.hack
module test.foo;
<<__EntryPoint>>
public function test_bar(): void {
    $x = new Bar(); // ok
}

```


## Experimental: Accessing symbols across packages

Sometimes, one may want to check at runtime whether a certain package is in the active deployment, and if so, behave differently. To do so, you can use a new `package` syntax. `package <pkgname>` desugars down to `package_exists(<pkgname>)`, and the typechecker intelligently allows you to access symbols within certain blocks when conditioned on `package` syntax. To access cross package syntax, enable the unstable feature "package".

```hack no-extract
module test.foo;
class TestFoo implements IFoo {
    //...
    public function testMethod(): void {}
}
```

```hack no-extract
<<file:__EnableUnstableFeatures("package")>>
module prod.bar;
interface IFoo {
    //...
}
class Foo implements IFoo {
    //...
}

class FooLoader {
   public async function genLoad(): Awaitable<IFoo> {
        if (package test) {
            // Typechecker allows this because it's within a if package block.
            return new TestFoo();
        } else {
            return new Foo();
        }
    }
}
```

You may also want to mark an entire method or function as cross package, which asserts that the method is only called when a package is loaded. You can do so by marking a method with the `<<__CrossPackage()>>` attribute.

```hack no-extract
<<file:__EnableUnstableFeatures("package")>>
module prod.bar; // in package prod
class Foo {
  <<__CrossPackage("test")>>
  public function getTest(): TestFoo {
    return new TestFoo();
  }
}
```
A cross package method can reference types from the package it requires. Calling a cross package method at runtime may lead to undefined class errors,  if the intended symbols are not in an active deployment. At typecheck time, cross package methods can only be called when you statically know a package is loaded (via a package loaded expression, for example).

```hack no-extract
<<file:__EnableUnstableFeatures("package")>>
public function test(): void {
    $x = new Foo();
    $y = null;
    if (package test) {
        $y = $x->getTest();
        $y->testMethod(); // ok, package test is loaded
    }
    $z = $x->getTest(); // error, cannot call a cross package method getTest since it requires package test to be loaded.

    // error, $y is in a different package unaccessible by this package, so is opaque.
    // please wrap this in a package statement
    $y?->testMethod();
}
```

If a package loaded expression is the only expression in an invariant statement, then for the rest of the scope of the current block, that package is accessible.
```hack no-extract
invariant(package test, "Test package is loaded");
$x = new TestFoo();
```
You can nest package statements to access multiple loaded packages at once.

```hack no-extract
if (package foo) {
  if (package bar) {
    // can access foo or bar
  } else {
    // can access foo
  }
}
```
## Runtime behavior

Cross package methods are primarily a typechecker feature, in that they allow you to express types and prove to the typechecker that a method exists. At runtime, the set of loaded packages is determined primarily by the active deployment (or the deployment matching the current request in non repo-authoritative mode). At runtime, `package foo` desugars to `package_exists(“foo”)`, which, just like other `*_exists` functions, returns true if "foo" is in the active deployment. `foo` is a static string in this context, and we throw a naming error at typecheck time if `foo` is not a valid package name. At runtime, calling `package_exists` or using `package` syntax on a non existent package will simply return false.

## Inheritance and subtyping

Note that you cannot implement an interface or extend a class from a different package, unless the current package includes that package (otherwise, you wouldn't know whether the class you were extending always existed).
```hack no-extract
module prod.foo;
class FooBad extends TestFoo {} // error, TestFoo is in package test. prod.foo is in package prod, which does not include test.
```

A method or function being cross package is **covariant** on its type. This means that regular, non cross-package functions are subtypes of cross package functions, but not vice versa. This also means that a non cross package method can override a cross package method, but not the other way around. Semantically, you can think of the Cross Package attribute as an extra argument on the method or function, as it requires a specific condition to be true before allowing the function to be called.

```hack no-extract
<<file:__EnableUnstableFeatures("package")>>
module prod.foo;
class FooLoader {
  public function get(): IFoo {
    return new Foo();
  }
}

class FooLoaderBad extends FooLoader {
  // error, cannot override non cross package method with cross package one.
  <<__Override, __CrossPackage("test")>>
  public function get(): IFoo {
    return new TestFoo();
  }
}
```
