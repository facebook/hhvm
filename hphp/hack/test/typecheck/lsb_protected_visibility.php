<?hh // strict

class A {
  <<__LSB>> protected static string $y = "!";

  public static function foo(): string {
    return static::$y; // ok
  }
}

class B extends A {
  public static function foo(): string {
    return static::$y; // ok
  }
}
