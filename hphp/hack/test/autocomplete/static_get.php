<?hh

class Foo {
  const X = 1;

  static int $x = 2;

  public static function f() {
    Foo::AUTO332  // autocomplete output has no defined order
  }
}
