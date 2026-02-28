# Type Erasure

Parameterized types with generics give you type safety without runtime
checks.

Generic type information is not available at runtime: this is called
"erasure". If you need this information, consider using [reified
generics](/hack/reified-generics/reified-generics).

```hack
function takes_ints(vec<int> $items): vec<int> {
  return $items;
}
```

When the type checker runs, a call `takes_ints(vec["hello"])` will
produce a type error. At runtime, since the parameter `int` is erased, we
only check that the argument and return type is a `vec`.

## Awaitable Types

For async functions, Hack will also enforce the wrapped return type at
runtime. For example, the following function will produce a runtime
error:

```hack error
async function my_foo(): Awaitable<int> {
  return "not an int";
}
```

## Erasure Limitations

Erasure prevents you using a generic type parameter `T` in the
following situations:

 * Creating instances: `new T()`
 * Calling static methods: `T::aStaticMethod()`
 * Type checks: `is T`
 * As the type of a static property.
 * As the type of the exception in a catch block: `catch (T $exception)`

For passing around class names for instantiation, Hack provides
[`classname<T>`](/hack/built-in-types/classname) that extends the
representation of `Foo::class`.
