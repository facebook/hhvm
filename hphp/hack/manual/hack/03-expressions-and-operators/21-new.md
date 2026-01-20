# New

The `new` operator allocates memory for an object that is an instance of the specified class.  The object is initialized by calling the
class's [constructor](/hack/classes/constructors) passing it the optional argument list, just like a function call. If the class has no
constructor, the constructor that class inherits (if any) is used.  For example:

```hack
class Point {
  private static int $pointCount = 0; // static property with initializer
  private float $x; // instance property
  private float $y; // instance property

  public function __construct(num $x = 0, num $y = 0) { // instance method
    $this->x = (float)$x; // access instance property
    $this->y = (float)$y; // access instance property
    ++Point::$pointCount; // include new Point in Point count
  }
}

function main(): void {
  $p1 = new Point(); // create Point(0.0, 0.0)
  $p2 = new Point(5, 6.7); // create Point(5.0, 6.7)
}
```

The result is an object of the type specified.

The `new` operator may also be used to allocate memory for an instance of a [classname](/hack/built-in-types/classname) type; for example:

```hack
final class C {}
function f(classname<C> $clsname): void {
  $w = new $clsname();
}
```

Any one of the keywords `parent`, `self`, and `static` can be used between the `new` and the constructor call, as follows. From within a
method, the use of `static` corresponds to the class in the inheritance context in which the method is called. The type of the object
created by an expression of the form `new static` is
[`this`](/hack/built-in-types/this). See [scope resolution](/hack/expressions-and-operators/scope-resolution) for a discussion of `parent`,
`self`, and `static` in this context.
