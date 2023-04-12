It is often useful to refer to a function
(or a [method](https://docs.hhvm.com/hack/classes/methods))
without actually calling it&mdash;for example,
as an argument for functions like `Vec\map`.

**Note:** The following syntax is only supported since HHVM 4.79. For older HHVM
versions, see [old syntax](#old-syntax) below.

To refer to a top-level (global) function named `foo`, you can write:

```Hack no-extract
foo<>; // a reference to global function 'foo'
```

You can think of it like it’s a function call with empty generics, but the list
of arguments has been omitted (the missing parentheses).

The following example stores a function reference in a variable and later calls
the function. Note that the type checker keeps track of the
[function's type](/hack/built-in-types/function)
and correctly checks all calls.

```Hack no-extract
function foo(string $x): bool { ... }

$x = foo<>;     // $x:(function(string): bool)
$y = $x('bar'); // $y:bool
$_ = $x(-1);    // error!
```

This syntax supports namespaced names the same way you would refer to them as
part of a function call, so the following function references are all
equivalent:

```Hack no-extract
$fun = \Foo\Bar\Baz\derp<>;
```

```Hack no-extract
namespace Foo;
$fun = Bar\Baz\derp<>;
```

```Hack no-extract
use namespace Foo\Bar\Baz;
$fun = Baz\derp<>;
```

```Hack no-extract
use function Foo\Bar\Baz\derp;
$fun = derp<>;
```

## Static methods

Similarly, you can refer to a static method `bar` on a class `MyClass` by using
the familiar method call syntax, without providing the call arguments. Just
append type arguments (like `<>`) to the function call receiver
(like `MyClass::bar`).

```Hack no-extract
MyClass::bar<>; // a reference to static method 'MyClass::bar'
```

- **Private/protected** methods can be referenced using this syntax as long as
  they are accessible to you in your local scope (the scope where the reference
  is created). The returned reference can then be called from any scope.
- **Abstract** static methods cannot be referenced. Such methods cannot be
  called, for they have no implementation. Invoking a hypothetical reference to
  one would also be an error, so we simply don’t allow a reference to be
  created.
- In traits and other classes that are not marked `final`, you cannot use
  `self::` in a reference. This is to avoid ambiguity and confusion around what
  `self` actually refers to when the method is called (it depends on the static
  context of a call, whereas function references can never receive the context
  information because the target is only resolved once).
- Also, `parent::` is never supported.

## Generics

If you wish to pass along explicit generic arguments, either as a hint to the
type checker, or in the case of a function with reified type parameters when
they are required, that is also supported:

```Hack no-extract
i_am_erased<int, _>; // erased generics (note wildcard)
i_am_reified<C<string>>; // a reified generic
```

- Keep in mind that generics, if any, still must be provided at the location
  where the function reference is created, rather than where it is used/invoked.
- The arity of the type argument list, if it is non-empty (i.e. not `<>`), must
  match the declaration, just like for an ordinary function call.
- The special wildcard specifier `_` may be provided in place of any or all
  erased (non-reified) generic arguments if you want Hack to infer a type
  automatically based on the type parameter’s constraints. Again this is the
  same as for ordinary function calls.

Example with erased generics:

```Hack no-extract
function fizz<Ta as num, Tb>(Ta $a, Tb $b): mixed { ... }

$x = fizz<int, string>; // OK
$x(4, 'hello');
$x(-1, false); // error!

$y = fizz<>;
$y(3.14, new C()); // also OK
$y('yo', derp()); // error!

// OK as well
$z = fizz<int, _>;
$z = fizz<_, string>;
$z = fizz<_, _>;

// these all have errors!
fizz<_>;
fizz<string, _>;
fizz<string, int>;
```

Example with [reified generics](/hack/generics/reified-generics):

```Hack no-extract
function buzz<reify T as arraykey>(T $x): mixed { ... }

$w = buzz<int>; // OK
$w(42);
$w("goodbye"); // error!

// these all have errors!
buzz<>;
buzz<_>;
buzz<mixed>;
```

## Introspection

Function references can be cached in APC (the name will be resolved again when
they are retrieved) or passed to memoized functions. However, other
serialization formats are not supported.

Internally, function/method references are represented using special data types
that are intended to be opaque. This means they cannot (or should not) be cast
directly to a string or another type, or be accessed in any other way besides
calling them.

If you need to determine what a function reference is pointing to, e.g. for
use in logging messages, and you know enough about the expected input and
output formats, HHVM provides the following helpers (but note they are not
well supported and may change):

* `HH\is_fun`
* `HH\fun_get_function`
* `HH\is_class_meth`
* `HH\class_meth_get_class`
* `HH\class_meth_get_method`

Be very careful and deliberate when using these, as they are loosely typed but
will throw exceptions for bad arguments.

<span data-nosnippet class="fbOnly fbIcon">In Meta's WWW repository, prefer using higher
level wrappers such as the `HackCallable` class, or `ReflectionFunctionProdUtils` outside
of intern code.</span>

## Old syntax

Before HHVM 4.79, there was no special syntax for function references. However,
the built-in functions `fun` and `class_meth` can be used for the same purpose.
For HHVM versions that support both options, the returned function references
are identical, e.g. `foo<>` is equivalent to `fun('foo')`.

There is also no equivalent syntax for referencing non-static methods. For
those, use the built-in functions `inst_meth` and `meth_caller`, or use an
[anonymous function](anonymous-functions) instead:

```Hack no-extract
class C {
  public function foo(): void {}
}

$obj = new C();
$fun1 = inst_meth($obj, 'foo');
$fun2 = () ==> $obj->foo();
// calling $fun1() is equivalent to calling $fun2()
```
