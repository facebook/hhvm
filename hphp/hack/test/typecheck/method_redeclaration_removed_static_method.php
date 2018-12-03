<?hh // strict

trait T {
  public static function f(): void {}
}

class C {
  use T;

  public static function g(): void = T::f;
}

function f(): void {
  C::f();
}
