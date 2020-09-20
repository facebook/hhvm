<?hh // strict

class A {
  <<__LSB>> private static int $x = 0;
}

class B extends A {
  public static function foo(): int {
    return static::$x;
  }
}
