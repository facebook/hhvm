<?hh

class A {
  <<__LSB>> public static int $x = 0;
}

class B extends A {
  public static function foo(): int {
    return parent::$x;
  }
}
