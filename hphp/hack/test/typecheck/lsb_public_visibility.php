<?hh

class A {
  <<__LSB>> public static int $x = 0;

  public static function foo(): int {
    return static::$x; // ok
  }
}

class B extends A {
  public static function bar(): int {
    return static::$x; // ok
  }
}

class C {
  public static function baz(): int {
    return A::$x; // ok
  }

  public static function qux(): int {
    return B::$x; // ok
  }
}
