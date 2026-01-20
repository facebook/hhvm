# Constructors

A constructor is a specially named instance method that is used to initialize the instance immediately after it has been created.  A
constructor is called by the [`new` operator](/hack/expressions-and-operators/new).  For example:

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

function demo(): void {
  $p1 = new Point(2.3);
}
```

A constructor has the name `__construct`.  As such, a class can have only one constructor.  (Hack does *not* support method overloading.)

When `new Point` causes the constructor to be called, the argument 2.3 maps to the parameter `$x`, and the default value 0 is mapped to
the parameter `$y`.  The constructor body is then executed, which results in the instance properties being initialized and the Point count
being incremented.  Note that a constructor may call any *private* method in its class, but no public methods.

A constructor does not require a return type, but if one is included, it must be `void`.

## Constructor parameter promotion

If you have created a class in Hack, you have probably seen a pattern like this:

```hack
final class User {
  private int $id;
  private string $name;

  public function __construct(
    int $id,
    string $name,
  ) {
    $this->id = $id;
    $this->name = $name;
  }
}
```

The class properties are essentially repeated multiple times: at declaration, in
the constructor parameters and in the assignment. This can be quite cumbersome.

With *constructor parameter promotion*, all that repetitive boilerplate is
removed.

```hack
final class User {
  public function __construct(
    private int $id,
    private string $name,
  ) {}
}
```

All you do is put a visibility modifier in front of the constructor parameter
and everything else in the previous example is done automatically, including the
actual creation of the property.

**Note:** Promotion can only be used for constructor parameters with name and
type that exactly match the respective class property. For example, we couldn't
use it in the `Point` class above because we wanted the properties to have type
`float`, so any arithmetic coordinate value can be represented, yet we wanted
the constructor parameters to have type `num`, so either integer or
floating-point values can be passed in.

Don't hesitate to &ldquo;un-promote&rdquo; a constructor parameter if it later
turns out that a different internal data representation would be better. For
example, if we later decided to store `$name` in a structured form instead of a string, we could easily make that change while keeping the public-facing
constructor parameters unchanged (and therefore backwards-compatible).

```hack no-extract
final class User {
  private ParsedName $name;

  public function __construct(
    private int $id,
    string $name,
  ) {
    $this->name = parse_name($name);
  }
}
```

### Rules

* A modifier of `private`, `protected` or `public` must precede the parameter
  declaration in the constructor.
* Other, non-class-property parameters may also exist in the constructor, as
  normal.
* Type annotations must go between the modifier and the parameter's name.
* The parameters can have a default value.
* Other code in the constructor is run **after** the parameter promotion
  assignment.

```hack
final class User {
  private static dict<int, User> $allUsers = dict[];
  private int $age;

  public function __construct(
    private int $id,
    private string $name,
    // Promoted parameters can be combined with regular non-promoted parameters.
    int $birthday_year,
  ) {
    $this->age = \date('Y') - $birthday_year;
    // The constructor parameter promotion assignments are done before the code
    // inside the constructor is run, so we can use $this->id here.
    self::$allUsers[$this->id] = $this;
  }
}
```
