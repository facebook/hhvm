# Cross Package Usage

Symbols in different packages are isolated by default. When a package explicitly includes another, it can access all symbols from the included package.

## Basic Package Inclusion

### The `includes` Field

The `includes` field in `PACKAGES.toml` grants access to symbols from included packages:

```toml
[packages]

[packages.production]
include_paths = ["//flib/prod/"]

[packages.test]
include_paths = ["//flib/test/"]
includes = ["production"] # test package includes production package

[deployments]
[deployments.production]
packages = ["production"]

[deployments.test]
packages = ["test", "production"] # Since test includes production, both must be deployed
```

With this configuration, the test package can access all production package symbols:

```hack no-extract
//// flib/prod/Bar.php
<?hh
// This file belongs to the production package
class Bar {
  public function productionMethod(): string {
    return "production";
  }
}
```

```hack no-extract
//// flib/test/TestBar.php
<?hh
// This file belongs to the test package
class TestBar {
    public function useProductionCode(): void {
        $bar = new Bar(); // OK: test includes production
        $result = $bar->productionMethod(); // OK: can call production methods
    }
}
```

### Directional Inclusion

Package inclusion is **not bidirectional**. A including B does not allow B to access A's symbols:

```hack no-extract
//// flib/test/Foo.php
<?hh
// This file belongs to the test package
class Foo {
  public function testMethod(): void {}
}
```

```hack no-extract
//// flib/prod/BadCall.php
<?hh
// This file belongs to the production package
public function bad_call(): void {
  // $x = new Foo();
  // ERROR: Typechecker error - Foo is in the test package,
  // which is not included by the production package
}
```

At runtime, using code from packages not in the active deployment will fail. Currently, this is only true for Repo-authoritative mode.

### Non-Transitive Inclusion

When package A includes package B, and package B includes package C, package A **cannot** automatically access package C through package B. A must explicitly include C:

```toml
[packages.core]
include_paths = ["//flib/core/"]

[packages.utils]
include_paths = ["//flib/utils/"]
includes = ["core"]  # utils can access core

[packages.application]
include_paths = ["//flib/app/"]
includes = ["utils"]  # application can access utils, but NOT core
# To access core, application must explicitly list it:
# includes = ["utils", "core"]
```

Explicit transitivity prevents types from escaping their boundaries and clarifies dependencies.

## Conditional Cross-Package Access

### The `package` Expression

The `package <packagename>` expression returns whether a package is loaded. In an `if` statement, the typechecker allows symbol access to the checked package within the conditional block:

```hack no-extract
// flib/test/TestFoo.php - in test package
<?hh
class TestFoo implements IFoo {
  public function testMethod(): void {}
}
```

```hack no-extract
// flib/prod/Foo.php - in production package
<?hh
interface IFoo {}

class Foo implements IFoo {}

class FooFactory {
  public function getFoo(): IFoo {
    if (package test) {
      // Typechecker allows this because test package is confirmed loaded here
      return new TestFoo();
    } else {
      return new Foo();
    }
  }
}
```

### Nested Expressions

Nest `package` expressions to access multiple packages:

```hack no-extract
if (package foo) {
  if (package bar) {
    // can access both foo and bar
  } else {
    // can access only foo
  }
  // can access only foo
}
```

**Note:** Package expressions are currently disallowed inside `invariant()` calls due to unexpected typechecker complexity.

### Runtime Behavior

- `package foo` desugars to `package_exists("foo")`
- Returns `true` if package is in the active deployment
- If the active deployment is *unset*, returns `true` for all existent packages
- Const-folded in repo-authoritative mode for zero runtime cost

## Cross-Package Methods

### The `__RequirePackage` Attribute

Mark methods/functions with `<<__RequirePackage()>>` to require a specific package:

```hack no-extract
// flib/prod/Foo.php - in production package
<?hh
class Foo {
  <<__RequirePackage("test")>>
  public function getTestFoo(): TestFoo {
    return new TestFoo();
  }
}
```

Cross-package methods can reference types from the required package in their body and signature, but can only be called where the package is confirmed loaded:

```hack no-extract
public function prod_func(): void {
  $x = new Foo();
  // error: cannot call cross-package method when `test` is not guaranteed to be loaded
  $x->getTestFoo();

  if (package test) {
    $x->getTestFoo(); // OK: `test` is loaded
  }
}

<<__RequirePackage("test")>>
public function other_prod_func(): void {
  $x = new Foo();
  $x->getTestFoo(); // OK: `test` is required via __RequirePackage
}
```

### Benefits

- **Type safety**: Typechecker enforces that calls only occur when the package is loaded
- **Clear signatures**: Cross-package types are allowed in parameters and return types
- **Type opacity**: Required package types are opaque outside of loaded contexts

### Runtime Behavior

Cross-package methods desugar to `assert(package_exists("pkg"))` at the start of the method:
- In repo-authoritative mode, these are const-folded away for zero runtime cost
- If the active deployment is *unset*, these assertions pass for all existent packages

## Inheritance and Subtyping

### Extending Cross-Package Types

Cannot extend/implement types from another package unless it's included:

```hack no-extract
// flib/prod/Bad.php - in production package

// error: TestFoo is in `test`, which the production package doesn't include
class FooBad extends TestFoo {}
```

### Method Covariance

Cross-package is **covariant**:
- Regular methods are subtypes of cross-package methods
- Regular methods can override cross-package methods
- Cross-package methods **cannot** override regular methods

```hack no-extract
class FooFactory {
  public function get(): IFoo {
    return new Foo();
  }
}

class FooFactoryBad extends FooFactory {
  // error: cannot override non cross-package method with cross-package method
  <<__Override, __RequirePackage("test")>>
  public function get(): IFoo {
    return new TestFoo();
  }
}
```

## Testing and Reflection

### Testing

Cross-package methods are testable/mockable like regular methods. The required package must be available in the test environment or exceptions may be triggered per the above specifications.

### Reflection

`__RequirePackage` is accessible via `ReflectionFunction::getAttributes()`.

## Typechecker Enforcement Exceptions

Temporary exceptions support legacy build behaviors where certain symbol references don't cause implicit inclusion in the legacy build system.

**Bypassed contexts** (migration only):
- **Type aliases**: `type MyType = ForeignPackageType`
- **Type hints**: Parameters and return types referencing foreign package types
- **Generics**: Reified and erased generics like `new C<ForeignPackageType>()`
- **Function types**: `function(ForeignPackageType): int`
- **Function pointers**: `ForeignPackageClass::method<>()`
- **Attributes**: `<<ForeignPackageClassAttribute()>>`
- **::class expressions**: `ForeignPackageClass::class`
- **Type constants**: Reifiable and non-reifiable type constants
- **Shape, vec, and dict types**: `shape('foo' => ForeignPackageType)`, `vec<ForeignPackageType>`, `dict<string, ForeignPackageType>`
- **is/as expressions**: `$x is ForeignPackageType`, `$x as ForeignPackageType`

These exceptions are intended to be temporary and may be removed in future versions as codebases enable.
