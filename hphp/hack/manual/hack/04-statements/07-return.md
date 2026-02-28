# Return

A `return` statement can only occur inside a function, in which case, it causes that function to terminate normally.  The function can
optionally return a single value (but one which could contain other values, as in a tuple, a shape, or an object of some user-defined
type), whose type must be compatible with the function's declared return type.  If the `return` statement contains no value, or there
is no `return` statement (in which case, execution drops into the function's closing brace), no value is returned.  For example:

```hack
function average_float(float $p1, float $p2): float {
  return ($p1 + $p2) / 2.0;
}

type IdSet = shape('id' => ?string, 'url' => ?string, 'count' => int);
function get_IdSet(): IdSet {
  return shape('id' => null, 'url' => null, 'count' => 0);
}

class Point {
  private float $x;
  private float $y;
  public function __construct(num $x = 0, num $y = 0) {
    $this->x = (float)$x; // sets private property $x
    $this->y = (float)$y; // sets private property $y
  } // no return statement
  public function move(num $x = 0, num $y = 0): void {
    $this->x = (float)$x; // sets private property $x
    $this->y = (float)$y; // sets private property $y
    return; // return nothing
  }
  // ...
}
```

However, for an async function having a `void` return type, an object of type `Awaitable<void>` is returned.  For an async function,
the value having a non-`void` return type, the return value is wrapped in an object of type `Awaitable<T>` (where `T` is the type of
the return value), which is returned.

Returning from a constructor behaves just like returning from a function having a return type of `void`.

The value returned by a [generator function](/hack/expressions-and-operators/yield) must be the literal `null`.  A `return` statement
inside a generator function causes the generator to terminate.

A return statement must not occur in a finally block or in a function declared [`noreturn`](/hack/built-in-types/noreturn).
