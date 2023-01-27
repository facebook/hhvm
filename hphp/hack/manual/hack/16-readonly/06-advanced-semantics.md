This page lists some more complicated interactions and nuances with readonly.

## `readonly (function (): T)` versus `(readonly function(): T)`: references vs. objects
A `(readonly function(): T)` may look very similar to a `readonly (function(): T)`, but they are actually different. The first denotes a readonly closure object, which at definition time, captured readonly values. The second denotes a readonly **reference** to a regular, mutable closure object:

```Hack
function readonly_closures_example2<T>(
  (function (): T) $regular_f,
  (readonly function(): T) $ro_f,
) : void {
  $ro_regular_f = readonly $regular_f; // readonly (function(): T)
  $ro_f; // (readonly function(): T)
  $ro_ro_f = readonly $ro_f; // readonly (readonly function(): T)
}
```

Since calling a mutable closure object can modify itself (and its captured values), a readonly reference to a regular closure **cannot** be called.

```Hack error
function readonly_closure_call<T>(
  (function (): T) $regular_f,
  (readonly function(): T) $ro_f,
) : void {
  $ro_regular_f = readonly $regular_f; // readonly (function(): T)
  $ro_regular_f(); // error, $ro_regular_f is a readonly reference to a regular function
}
```

But a readonly closure object can have readonly references and call them, since they cannot modify the original closure object on call:

```Hack error
function readonly_closure_call2<T>(
  (function (): T) $regular_f,
  (readonly function(): T) $ro_f,
  ) : void {
    $ro_regular_f = readonly $regular_f; // readonly (function(): T)
    $ro_regular_f(); // error, $ro_regular_f is a readonly reference to a regular function
    $ro_ro_f = readonly $ro_f; // readonly (readonly function(): T)
    $ro_ro_f(); // safe
  }
```

## Converting to non-readonly
Sometimes you may encounter a readonly value that isn’t an object (e.g.. a readonly int, due to the deepness property of readonly). In those cases, instead of returning a readonly int, you’ll want a way to tell Hack that the value you have is actually a value type. You can use the function `HH\Readonly\as_mut()` to convert any primitive type from readonly to mutable.

Use `HH\Readonly\as_mut()` strictly for primitive types and value-type collections of primitive types (i.e. a vec of int).

```Hack

class Foo {
  public function __construct(
    public int $prop,
  ) {}

  public readonly function get() : int {
    $result = $this->prop; // here, $result is readonly, but its also an int.
    return \HH\Readonly\as_mut($this->prop); // convert to a non-readonly value
  }
}
```
