When the left-hand operand of operator `::` is an enumerated type, the right-hand operand must be the name of an enumeration constant
within that type.  For example:

```Hack
enum ControlStatus: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 3;
}

function main(ControlStatus $p1): void {
  switch ($p1) {
  case ControlStatus::Stopped:
    // ...
    break;
  case ControlStatus::Stopping:
    // ...
    break;
  default:
    break;
  }
}
```

From inside or outside a class or interface, operator `::` allows the selection of a constant. From inside or outside a class, this
operator allows the selection of a static property, static method, or instance method.  For example:

```Hack
final class MathLibrary {
  const PI = 3.1415926;
  public static function sin(float $val): float {
    return 0.0; // stub
  }
}

function use_it(): void {
  $radius = 3.4;
  $area = MathLibrary::PI * $radius * $radius;
  $v = MathLibrary::sin(2.34);
}
```

From within a class, `::` also allows the selection of an overridden property or method.

From within a class, `self::m` refers to the member `m` in that class. For example:

```Hack
class Point {
  private static int $pointCount = 0;
  public static function getPointCount(): int {
    return self::$pointCount;
  }
}
```

From within a class, `parent::m` refers to the closest member `m` in the base-class hierarchy, not including the current class.  For example:

```Hack
class MyRangeException extends Exception {
  public function __construct(string $message) {
    parent::__construct('MyRangeException: '.$message);
  }
}
```

From within a method, `static::m` refers to the static member `m` in the class that corresponds to the class inheritance context in
which the method is called. This allows *late static binding*. Consider the following scenario:

```Hack
class Base {
  public function b(): void {
    static::f();  // calls the most appropriate f()
  }
  public static function f(): void {
    //...
  }
}

class Derived extends Base {
  public static function f(): void {
    // ...
  }
}

function demo(): void {
  $b1 = new Base();
  $b1->b(); // as $b1 is an instance of Base, Base::b() calls Base::f()
  $d1 = new Derived();
  $d1->b(); // as $d1 is an instance of Derived, Base::b() calls Derived::f()
}
```

The value of a scope-resolution expression ending in `::class` is a string containing the fully qualified name of the current
class, which for a static qualifier, means the current class context.  A class identifier followed by `::class` results in a
constant whose value has the [`classname` type](../built-in-types/classname.md) parameterized by the class.  For example:

```Hack
namespace NS_cn;
class C1 {
  // ...
}
class C2 {
  public static classname<C1> $p1 = C1::class;
  public static function f(?classname<C1> $p): void {}
  public static vec<classname<C1>> $p2 = vec[C1::class];
}
```
