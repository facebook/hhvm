<?hh

class A {
  <<__LSB>> protected static string $y = "!";
}

class B extends A {
}

class C {
  public static function foo(): string {
    return B::$y;
  }
}
