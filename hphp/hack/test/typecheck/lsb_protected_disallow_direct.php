<?hh

class A {
  <<__LSB>> protected static string $y = "!";
}

class B {
  public static function foo(): string {
    return A::$y;
  }
}
