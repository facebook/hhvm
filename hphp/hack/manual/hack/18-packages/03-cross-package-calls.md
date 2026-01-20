# Cross Package Usage

Symbols in different packages are isolated by default. When a package explicitly includes another, it can access all symbols from the included package.

## Basic Package Inclusion

### The `includes` Field

The `includes` field in `PACKAGES.toml` grants access to symbols from included packages:

```toml
[packages]

[packages.production]
include_paths = ["//prod/"]

[packages.test]
include_paths = ["//test/"]
includes = ["production"] # test package includes production package

[deployments]
[deployments.production]
packages = ["production"]

[deployments.test]
packages = ["test", "production"] # Since test includes production, both must be deployed
```

With this configuration, the test package can access all production package symbols:

```hack no-extract
//// prod package
<?hh
class Bar {
  public function productionMethod(): string {
    return "production";
  }
}
```

```hack no-extract
//// test package
<?hh
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
//// test package
<?hh
class Foo {
  public function testMethod(): void {}
}
```

```hack no-extract
//// prod package
<?hh
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
include_paths = ["//core/"]

[packages.utils]
include_paths = ["//utils/"]
includes = ["core"]  # utils can access core

[packages.application]
include_paths = ["//app/"]
includes = ["utils"]  # application can access utils, but NOT core
# To access core, application must explicitly list it:
# includes = ["utils", "core"]
```

Explicit transitivity prevents types from escaping their boundaries and clarifies dependencies.

## Conditional Cross-Package Access

### The `package` Expression

The `package <packagename>` expression returns whether a package is loaded. In an `if` statement, the typechecker allows symbol access to the checked package within the conditional block:

```hack no-extract
//// test package
<?hh
class TestFoo implements IFoo {
  public function testMethod(): void {}
}
```

```hack no-extract
//// prod package
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

The `__RequirePackage` and `__SoftRequirePackage` attributes enable methods and functions to be called across package boundaries with specific constraints and guarantees.

Note that unlike `__PackageOverride`, these attributes **do not** move the definition into the required package.

### Overview

Hack provides two attributes for enabling cross-package method access: `__RequirePackage` and `__SoftRequirePackage`. Both allow methods and functions to be called from packages other than the one they're defined in, subject to specific constraints.

| Aspect | `__RequirePackage` | `__SoftRequirePackage` |
|--------|-------------------|----------------------|
| **Caller Requirements** | Caller must have access to the specified package | Caller must have access to the specified package |
| **Body Access** | CAN access members of the required package | CANNOT access members of the required package |
| **Runtime Behavior** | Asserts that the package is available | Logs if the package is not available |

### Where These Methods Can Be Called

Methods with either attribute can only be called from:

1. **Within Package 'pkg':** From code directly in package 'pkg'
2. **Package Conditional Blocks:** Within `if (package pkg)` blocks
3. **Functions/Methods with Package Access:** From functions/methods that have `<<__RequirePackage('pkg')>>` or `<<__SoftRequirePackage('pkg')>>` (in the case of calling another `__Soft` function)

Note that package checks are _inclusion_ checks, not equality checks. If package A includes package B, and package B, a method with `__RequirePackage(B)` may be called from a context having access to package A .

```hack no-extract
// In production package
<<__RequirePackage('intern')>>
function requires_intern(): void {}

// Invalid call
function invalid_caller(): void {
  requires_intern(); // ERROR: cannot call without package guarantee
}

// Valid calls
<<__RequirePackage('intern')>>
function valid_caller(): void {
  requires_intern(); // OK: caller has __RequirePackage('intern')
}

function conditional_caller(): void {
  if (package intern) {
    requires_intern(); // OK: within package conditional
  }
}
```

### Package Inclusion Requirements

When using `<<__RequirePackage('a')>>` or `<<__SoftRequirePackage('a')>>` in package 'b', package 'a' must include package 'b'.

### `__RequirePackage`

Mark methods/functions with `<<__RequirePackage('package_name')>>` to require a specific package:

```hack no-extract
// In production package
<?hh
class Foo {
  <<__RequirePackage("intern")>>
  public function getInternFoo(): InternFoo {
    // Can access symbols from intern package in body
    intern_func();
    return new InternFoo();
  }
}
```

Within a `__RequirePackage('pkg')` method body, you can reference symbols from package 'pkg'. This feature enables gating functions meant only for use within a specific package.

**Runtime Behavior:** Desugars to `invariant(package pkg, '...')` at the start of the method. In repo-authoritative mode, these assertions are const-folded away for zero runtime cost.

### `__SoftRequirePackage`

`__SoftRequirePackage` is a migratory tool between no attribute and `__RequirePackage`, avoiding accidental assertions due to dynamic invocations.

Within the body, you may NOT reference symbols from the softly required package. This prevents accidental fatals from references to unincluded symbols. However, a function softly requiring a package can invoke other functions that softly require the same (or included) packages.

```hack no-extract
class ProdClass {
  <<__SoftRequirePackage('intern')>>
  public function softFunc(): void {
    intern_func(); // ERROR: cannot access intern in __SoftRequirePackage body
    $this->otherSoftFunc(); // OK: can call other soft-requiring methods
  }

  public function prodFunc(): void {
    $this->softFunc(); // ERROR: cannot access intern from prod package
  }

  <<__SoftRequirePackage('intern')>>
  public function otherSoftFunc(): void {}
}
```

**Runtime Behavior:** Desugars to `if(!(package pkg)) { log() }` at the start of the method.

**Optional Sample Rate:** Like `__Deprecated`, `__SoftRequirePackage` accepts an optional second argument for log sampling:

```hack no-extract
<<__SoftRequirePackage('intern', 1000)>> // Log 1 in 1000 calls
function soft_func(): void {}
```

When not provided, the sample rate defaults to 1 (log every call).

For detailed migration workflows using `__SoftRequirePackage`, see the [Migration Guide](/hack/packages/migration-guide).

## Method Overriding and Inheritance

### Override Rules

The general rule for overrides involving `__(Soft)RequirePackage` is that children must require **less than or equal to** their parents along two axes:

1. **Hard/Soft/No Attr:** A no attribute method can override a `__SoftRequirePackage` method, which can override a `__RequirePackage` method. The reverse is illegal.

2. **Package Inclusion:** If package A includes package B, a `__RequirePackage(B)` method can override a `__RequirePackage(A)` method. The reverse (or no relationship) is illegal.

This mirrors argument contravariance: you cannot define a hierarchy where invoking a child method via a parent class requires LESS than the child requires, failing to fulfill the child method's requirements.

### Override Examples

**Example 1: Valid - Removing RequirePackage**
```hack no-extract
class ProdClassRP {
  <<__RequirePackage('intern')>>
  public function bar(): void {
    intern_func();
  }
}

class ProdClassChild extends ProdClassRP {
  // OK: removing RequirePackage is valid
  <<__Override>>
  public function bar(): void {}
}
```

**Example 2: Invalid - Adding RequirePackage**
```hack no-extract
class ProdClass {
  public static function prodfun(): void {}
}

class ProdChildClass extends ProdClass {
  // ERROR: child requires more than parent
  <<__Override, __RequirePackage('intern')>>
  public static function prodfun(): void {}
}
```

**Example 3: Valid - Same Requirement**
```hack no-extract
class ProdClassRP {
  <<__RequirePackage('intern')>>
  public function qux(): void {}
}

class ProdClassRPChild extends ProdClassRP {
  <<__Override, __RequirePackage('intern')>>
  public function qux(): void {} // OK: same requirement
}
```

**Example 4: Invalid - Soft to Hard**
```hack no-extract
class ProdClassSRP {
  <<__SoftRequirePackage('intern')>>
  public function baz(): void {}
}

class ProdClassChild extends ProdClassSRP {
  // ERROR: Possibly statically acceptable, but dynamically risky
  <<__Override, __RequirePackage('intern')>>
  public function baz(): void {
    intern_func();
  }
}
```

**Example 5: Valid - Hard to Soft**
```hack no-extract
class ProdClassRP {
  <<__RequirePackage('intern')>>
  public function bar(): void {
    intern_func();
  }
}

class ProdClassChild extends ProdClassRP {
  // OK: Softening requirement is valid
  <<__Override, __SoftRequirePackage('intern')>>
  public function bar(): void {
    // Cannot call intern_func() here
    // Cannot invoke parent::bar() either
  }
}
```

### Constructor Inheritance

Constructors with `__RequirePackage` follow the same rules, with an additional consideration: overriding a constructor that requires a package with one that doesn't is typically impossible because `parent::__construct()` wouldn't be callable.

```hack no-extract
class ProdRPClass {
  <<__RequirePackage('intern')>>
  public function __construct() {
    // can only create ProdRPClass in intern package context
  }
}

class ProdChildClass extends ProdRPClass {
  <<__Override>>
  public function __construct(): void {
    parent::__construct(); // ERROR: calling parent requires intern package
  }
}
```

**ConsistentConstruct Implications:**

```hack no-extract
<<__ConsistentConstruct>>
class ProdRPClass {
  <<__RequirePackage('intern')>>
  public function __construct() {}

  public static function make(): this {
    return new static(); // ERROR: constructor requires intern
  }
}
```

If your constructor is marked `__ConsistentConstruct` and `__RequirePackage`, this implies the object cannot be constructed without that package being loaded. The typechecker does not currently infer access to the required package in other instance methods, but this may be enabled in the future.

### Function Pointers

The package and/or `__RequirePackage`-ness of a function is checked when creating a pointer to that function:

```hack no-extract
class ProdClass {
  <<__RequirePackage('intern')>>
  public static function prodRPfun(int $in): int {
    return $in;
  }

  public static function prodfun(): void {
    // ERROR: Cannot create function pointer to method requiring intern
    Vec\map(vec[1,2,3], ProdClass::prodRPfun<>);

    // Same as:
    Vec\map(vec[1,2,3], $i ==> ProdClass::prodRPfun($i)); // Also ERROR
  }

  <<__RequirePackage('intern')>>
  public static function prodRPfun2(): void {
    Vec\map(vec[1,2,3], ProdClass::prodRPfun<>); // OK: has intern access
  }
}
```

**Note:** This is currently one of the cases within the below "Typechecker Enforcement Exceptions" section. Full enforcement will be enabled after initial launch.

## Extending Cross-Package Types

Cannot extend/implement types from another package unless it's included:

```hack no-extract
// in production package

// error: TestFoo is in `test`, which the production package doesn't include
class FooBad extends TestFoo {}
```

## Testing and Reflection

### Testing

Cross-package methods are testable/mockable like regular methods. The required package must be available in the test environment or exceptions may be triggered per the above specifications.

### Reflection

`__RequirePackage` and `__SoftRequirePackage` are accessible via `ReflectionFunction::getAttributes()`.

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
