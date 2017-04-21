<?hh

class Foo {
  private static function f() {}

  public static function g(classname<Foo> $x) {
    $x::f();
  }
}
