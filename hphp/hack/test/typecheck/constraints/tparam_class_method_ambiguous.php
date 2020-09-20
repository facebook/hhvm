<?hh // strict

class A {}
class B extends A {}

interface I {
  public static function f(): A;
}

interface J extends I {
  public static function f(): B;
}

function expect_B(B $_): void {}

function test<T as J as I>(classname<T> $c): void {
  expect_B($c::f());
}
