# Num

The type `num` can represent any `int` or `float` value. This type can be useful when specifying the interface to a function. Consider the
following function declarations from the math library:

```hack no-extract
function sqrt(num $arg): float;
function log(num $arg, ?num $base = null): float;
function abs<T as num>(T $number): T;
function mean(Container<num> $numbers): ?float;
```

The square-root function `sqrt` takes a `num` and returns a `float`. The log-to-any-base function `log` takes a `num` and a nullable-of-`num`
and returns a `float`. The generic absolute-value function `abs` has one type parameter, `T`, which is constrained to having type `num` or a
subtype of `num`. `abs` takes an argument of type `T` and returns a value of the same type. The arithmetic-mean function `mean` takes a generic
type `Container`-of-type-`num` and returns a nullable-of-`float`.

Consider the following example:

```hack
class Point {
  private float $x;
  private float $y;
  public function __construct(num $x = 0, num $y = 0) {
    $this->x = (float)$x;
    $this->y = (float)$y;
  }
  public function move(num $x = 0, num $y = 0): void {
    $this->x = (float)$x;
    $this->y = (float)$y;
  }
  // ...
}
```

Internally, class `Point` stores the x- and y-coordinates as `float`s, but, for convenience, it allows any combination of `int`s and `float`s
to be passed to its constructor and method `move`.

When given a `num` value, to find out what type of value that `num` actually contains, use the `is` operator.

See the discussion of [type refinement](/hack/types/type-refinement).
