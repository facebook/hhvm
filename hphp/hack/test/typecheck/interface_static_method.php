<?hh // strict

interface A {
  public static function foo(): void;
}

final class B implements A {
  public static function foo(): void {}
}

interface C {
  require extends B;
}

function bar(): void {
  B::foo();
  C::foo();
}
