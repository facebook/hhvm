<?hh

class Foo {
  private static function f(): void {}

  public static function g(classname<Foo> $x): void {
    $x::f();
  }
}
