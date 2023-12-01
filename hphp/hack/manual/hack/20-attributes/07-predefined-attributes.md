The following attributes are defined:
* [__AcceptDisposable](#__acceptdisposable)
* [__AutocompleteSortText](#__autocompletesorttext)
* [__ConsistentConstruct](#__consistentconstruct)
* [__Deprecated](#__deprecated)
* [__Docs](#__docs)
* [__DynamicallyCallable](#__dynamicallycallable)
* [__DynamicallyConstructible](#__dynamicallyconstructible)
* [__EnableMethodTraitDiamond](#__enablemethodtraitdiamond)
* [__Enforceable](#__enforceable)
* [__EntryPoint](#__entrypoint)
* [__Explicit](#__explicit)
* [__LateInit](#__lateinit)
* [__LSB](#__lsb)
* [__Memoize](#__memoize)
* [__MemoizeLSB](#__memoizelsb)
* [__MockClass](#__mockclass)
* [__ModuleLevelTrait](#__moduleleveltrait)
* [__Newable](#__newable)
* [__Override](#__override)
* [__PHPStdLib](#__phpstdlib)
* [__ReturnDisposable](#__returndisposable)
* [__Sealed](#__sealed)
* [__Soft](#__soft)

## __AcceptDisposable

This attribute can be applied to a function parameter that has a type that implements interface `IDisposable` or `IAsyncDisposable`.

See [object disposal](../classes/object-disposal.md) for an example of its use.

## __AutocompleteSortText

When we are displaying autocomplete suggestions, we sort method suggestions
alphabetically. To override what text is used for sorting a specific method in
the autocomplete suggestions, use the `__AutocompleteSortText` attribute.

```Hack file:base.hack
class Dog {
  <<__AutocompleteSortText('!getName')>>
  public function getName(): string {...}

  public function getOwner(): string {...}

  public function getFriends(): vec<string> {...}

}
```

In the above example, autocomplete suggestions would typically show in the
order: `getFriends`, `getName`, `getOwner`

But since the `__AutocompleteSortText` is used, the order is: `getName`,
`getFriends`, `getOwner`

## __ConsistentConstruct

When a method is overridden in a derived class, it must have exactly the same number, type, and order of parameters as that in the base
class. However, that is not usually the case for constructors. Having a family of constructors with different signatures can cause a problem,
however, especially when using `new static`.

This attribute can be applied to classes; it has no attribute values.  Consider the following example:

```Hack
<<__ConsistentConstruct>>
class Base {
  public function __construct() {}

  public static function make(): this {
    return new static();
  }
}

class Derived extends Base {
  public function __construct() {
    parent::__construct();
  }
}

function demo(): void {
  $v2 = Derived::make();
}
```

When `make` is called on a `Derived` object, `new static` results in `Derived`'s constructor being called knowing only the parameter list
of `Base`'s constructor. As such, `Derived`'s constructor must either have the exact same signature as `Base`'s constructor, or the same
plus an ellipsis indicating a trailing variable-argument list.

## __Deprecated

If you mark a function with `__Deprecated`, the Hack typechecker will find all static invocations of that function and mark them as errors so that you can find those invocations and fix them as needed. For runtime invocations, any function marked with `__Deprecated` will still be called successfully, but with runtime logging so that you can find and fix those dynamic invocations later.

Consider the following example:

```Hack
<<__Deprecated("This function has been replaced by do_that", 7)>>
function do_this(): void { /* ... */ }
```

The presence of this attribute on a function has no effect, unless that function is actually called, in which case, for each call to that
function, HHVM raises a notice containing the text from the first attribute value.  The optional `int`-typed second attribute
value (in this case, 7) indicates a *sampling rate*.

Every 1/sampling-rate calls (as in, 1/7) to that function will raise a notice at runtime. If omitted, the default sampling rate is 1
(i.e. all calls raise notices).

To disable runtime notices, use a sampling rate of 0.

## __Docs

Associates a documentation URL with a type.

```Hack file:base.hack
<<__Docs("http://www.example.com/my_framework")>>
class MyFrameworkBaseClass {}
```

The IDE will include this URL when hovering over the
`MyFrameworkBaseClass` type name.

```Hack file:base.hack
class MyClass extends MyFrameworkBaseClass {}
```

Docs URLs are also inherited. Hovering over the `MyClass` type name
will also show the docs URL.

## __DynamicallyCallable

Allows a function or method to be called dynamically, based on a
string of its name. HHVM will warn on error (depending on
configuration) on dynamic calls to functions or methods without this attribute.

## __DynamicallyConstructible

Allows this class to be instantiated dynamically, based on a string of
its name. HHVM will warn on error (depending on configuration) on
dynamic instantiations of classes without this attribute.

## __EnableMethodTraitDiamond

This attribute can be applied to a class or trait to enable resolution of traits used along multiple paths.

See [using a trait](../traits-and-interfaces/using-a-trait.md) for an example of its use.


## __Enforceable

A type is _enforceable_ if it can be used in `is` and `as` expressions.  Examples of non-enforceable types are function types and erased (non-reified) generics.  The `__Enforceable` attribute is used to annotate abstract type constants so they can only be instantiated with enforceable types, and thus used in `is` and `as` expressions. The attribute restricts deriving type constants to values that are valid for a type test.

```Hack error
abstract class A {
  abstract const type Tnoenf;
  <<__Enforceable>>
  abstract const type Tenf;

  public function f(mixed $m): void {
    $m as this::Tenf; // OK

    $m as this::Tnoenf; // Hack error
  }
}

class B1 extends A {
  const type Tnoenf = (function (): void); // ok
  const type Tenf = (function (): void); // Hack error, function types cannot be used in type tests
}

class B2 extends A {
  const type Tnoenf = (function (): void); // ok
  const type Tenf = int; // ok
}
```

Similarly, the `__Enforceable` attribute can also be used to annotate reified generics, enabling the generic to be used in a type test expression.

## __Explicit

Requires callers to explicitly specify the value for a generic
type. Normally Hack allows generics to be inferred at the call site.

```Hack error
function values_are_equal<<<__Explicit>> T>(T $x, T $y): bool {
  return $x === $y;
}

function example_usage(int $x, int $y, string $s): void {
  values_are_equal<int>($x, $y);

  // Without <<__Explicit>>, this code would be fine, even though
  // it always returns false.
  values_are_equal($x, $s);
}
```

## __EntryPoint

A Hack program begins execution at a top-level function referred to as the *entry-point function*. A top-level function can be designated as such using this attribute, which
has no attribute values. For example:

```Hack
<<__EntryPoint>>
function main(): void {
  printf("Hello, World!\n");
}
```

Note: An entry-point function will *not* be automatically executed if the file containing such a function is included via require or the autoloader.

## __LateInit

Hack normally requires properties to be initialized, either with an
initial value on the property definition or inside the constructor.

`__LateInit` disables this check.

```Hack
class Foo {}

class Bar {
  <<__LateInit>> private Foo $f;

  public function trustMeThisIsCalledEarly(): void {
    $this->f = new Foo();
  }
}
```

**This is intended for testing**, where it's common to have a setup
function that initializes values.

Accessing a property that is not initialized produces a runtime error.

`__LateInit` can also be used with static properties.

```Hack
class Foo {}

class Bar {
  <<__LateInit>> private static Foo $f;

  public static function trustMeThisIsCalledEarly(): void {
    self::$f = new Foo();
  }
}
```

It may be clearer to write your code using a memoized static method
instead of a static property with `__LateInit`.

## __LSB

Marks this property as implicitly redeclared on all subclasses. This ensures each subclass has its own value for the property.

## __Memoize

The presence of this attribute causes the designated method to automatically cache each value it looks up and returns, so future calls with
the same parameters can be retrieved more efficiently. The set of parameters is hashed into a single hash key, so changing the type, number,
and/or order of the parameters can change that key. Functions with variadic parameters can not be memoized.

This attribute can be applied to functions and static or instance methods; it has no attribute values.  Consider the following example:

```Hack no-extract
class Item {
  <<__Memoize>>
  public static function get_name_from_product_code(int $productCode): string {
    if (name-in-cache) {
      return name-from-cache;
    } else {
      return Item::get_name_from_storage($productCode);
    }
  }
  private static function get_name_from_storage(int $productCode): string {
    // get name from alternate store
    return ...;
  }
}
```

`Item::get_name_from_storage` will only be called if the given product code is not in the cache.

The types of the parameters are restricted to the following: `null`, `bool`, `int`, `float`, `string`, any object type that implements
`IMemoizeParam`, enum constants, tuples, shapes, and arrays/collections containing any supported element type.

The interface type `IMemoizeParam` assists with memoizing objects passed to async functions.

You can clear the cache with `HH\clear_static_memoization`. This should only be used **in tests** where:
- the component being tested is meant to be immutable/idempotent for the entire request
- the test needs to cover multiple initial states, where only one would truly be reachable in a single request

NOTE: Putting the `__Memoize` attribute on a static method will cause it to bind
to the declaring class. When you do this, any uses of `static::` constructs to
retrieve definitions from subclasses can cause unexpected results (they will
actuually access the declaring class, similar to equivalent `self::` constructs).
Consider using `__MemoizeLSB` instead on static methods.

### Exceptions

Thrown exceptions are not memoized, showing by the increasing counter in this
example:

```Hack
class CountThrows {
  private int $count = -1;
  <<__Memoize>>
  public function doStuff(): void {
    $this->count += 1;
    throw new \Exception('Hello '.$this->count);
  }
}

<<__EntryPoint>>
function main(): void {
  $x = new CountThrows();
  for($i = 0; $i < 2; ++$i) {
    try {
      $x->doStuff();
    } catch (\Exception $e) {
      \var_dump($e->getMessage());
    }
  }
}
```

### Awaitables and Exceptions
As memoize caches an [Awaitable](../asynchronous-operations/awaitables) itself, this means that **if an async function
is memoized and throws, you will get the same exception backtrace on every
failed call**.

For more information and examples, see [Memoized Async Exceptions](../asynchronous-operations/exceptions#memoized-async-exceptions).

## __MemoizeLSB

This is like [<<__Memoize>>](#__memoize), but the cache has Late Static Binding. Each subclass has its own memoize cache.

You can clear the cache with `HH\clear_lsb_memoization`. This should only be used **in tests** where:
- the component being tested is meant to be immutable/idempotent for the entire request
- the test needs to cover multiple initial states, where only one would truly be reachable in a single request

## __MockClass
```yamlmeta
{
  "fbonly messages": [
    "Mock classes are intended for test infrastructure. They should not be added or used directly in Facebook's WWW repository."
  ]
}
```
Mock classes are useful in testing frameworks when you want to test functionality provided by a legitimate, user-accessible class,
by creating a new class (many times a child class) to help with the testing. However, what if a class is marked as `final` or a method in a
class is marked as `final`? Your mocking framework would generally be out of luck.

The `__MockClass` attribute allows you to override the restriction of `final` on a class or method within a class, so that a
mock class can exist.

```Hack no-extract
final class FinalClass {
  public static function f(): void {
    echo __METHOD__, "\n";
  }
}

// Without this attribute HHVM would throw a fatal error since you are trying
// to extend a final class. With it, you can run the code as you normally would.
// That said, you will still get Hack typechecker errors, since it does not
// recognize this attribute as anything intrinsic, but these can be suppressed.

/* HH_IGNORE_ERROR [2049] */
<<__MockClass>>
/* HH_IGNORE_ERROR [4035] */
final class MockFinalClass extends FinalClass {
  public static function f(): void {
    echo __METHOD__, "\n";

    // Let's say we were testing the call to the parent class. We wouldn't
    // be able to do this in HHVM without the __MockClass attribute.
    parent::f();
  }
}

<<__EntryPoint>>
function main(): void {
  $o = new MockFinalClass();
  $o::f();
}
```

Mock classes *cannot* extend types `vec`, `dict`, and `keyset`, or the Hack legacy types `Vector`, `Map`, and `Set`.

## __ModuleLevelTrait

Can be used on public traits.  The elements of a trait annotated with `<<__ModuleLevelTrait>>` are considered to belong to the module where the trait is defined, and can access other internal symbols of the module.  For more information see [Traits and Modules](../modules/traits.md).

## __Newable

This attribute is used to annotate reified type parameters to ensure that they are only instantiated with classes on which `new` can be safely called.  A common pattern, defining a function that creates instances of a class passed as type parameter, is:

```Hack
final class A {}

function f<<<__Newable>> reify T as A>(): T {
  return new T();
}
```

The class `A` must either be final (as in the example) or annotated with `__ConsistentConstruct`.  The `__Newable` attribute ensures that the function `f` is only be applied to a _non-abstract_ class, say `C`, while the `as A` constraint guarantees that the interface of the constructor of `C` is uniquely determined by the interface of the constructor of class `A`.  The generic type `T` must be reified so that the runtime has access to it, refer to [Generics: Reified Generics](../generics/reified-generics.md) for details.

A complete example thus is:

```Hack
<<__ConsistentConstruct>>
abstract class A {
  public function __construct(int $x, int $y) {}
}

class B extends A {}

function f<<<__Newable>> reify T as A>(int $x, int $y): T {
  return new T($x,$y);
}

<<__EntryPoint>>
function main(): void {
  f<B>(3,4);             // success, equivalent to new B(3,4)
}
```

Omitting either the `__Newable` attribute for `T`, or the `__ConsistentConstruct` for the abstract class `A` will result in a type-checker error.


## __Override

Methods marked with `__Override` must be used with inheritance.

For classes, `__Override` ensures that a parent class has a method
with the same name.

```Hack
class Button {
  // If we rename 'draw' to 'render' in the parent class,
  public function draw(): void { /* ... */ }
}
class CustomButton extends Button {
  // then the child class would get a type error.
  <<__Override>>
  public function draw(): void { /* ... */ }
}
```

For traits, `__Override` ensures that trait users have a method that
is overridden.

```Hack
class Button {
  public function draw(): void { /* ... */ }
}

trait MyButtonTrait {
  <<__Override>>
  public function draw(): void { /* ... */ }
}

class ExampleButton extends Button {
  // If ExampleButton did not have an inherited method
  // called 'draw', this would be an error.
  use MyButtonTrait;
}
```

It is often clearer to use constraints on traits instead. The above
trait could also be written like this.

```Hack
class Button {
  public function draw(): void { /* ... */ }
}

trait MyButtonTrait {
  // This makes the relationship with Button explicit.
  require extends Button;

  public function draw(): void { /* ... */ }
}

class ExampleButton extends Button {
  use MyButtonTrait;
}
```

## __PHPStdLib

This attribute tells the type checker to ignore a function or class,
so type errors are reported on any code that uses it.

This is useful when gradually deprecating PHP features.

`__PHPStdLib` only applies on `.hhi` files by default, but can apply
everywhere with the option `deregister_php_stdlib`.

## __ReturnDisposable

This attribute can be applied to a function that returns a value whose type implements interface `IDisposable` or `IAsyncDisposable`.

See [object disposal](../classes/object-disposal.md) for an example of its use.

## __Sealed

A class that is *sealed* can be extended directly only by the classes named in the attribute value list. Similarly, an interface that is sealed
can be implemented directly only by the classes named in the attribute value list. Classes named in the attribute value list can themselves be
extended arbitrarily unless they are final or also sealed. In this way, sealing provides a single-level restraint on inheritance.
For example:

```Hack
<<__Sealed(X::class, Y::class)>>
abstract class A {}

class X extends A {}
class Y extends A {}

<<__Sealed(Z::class)>>
interface I {}

class Z implements I {}
```

Only classes `X` and `Y` can directly extend class `A`, and only class `Z` can directly implement interface `I`.

## __Soft

Disable type enforcement. See [soft types](/hack/types/soft-types).
