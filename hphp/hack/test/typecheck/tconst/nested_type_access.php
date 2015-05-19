<?hh // strict

abstract class A {
  const type T = Container<int>;
}

class C {
  const type B as D = D;
  const type Test = this::B::X;

  public static function foo(): this::Test::T {
    return array();
  }
}

abstract class B {
  const type X = A;
}

class D extends B {}

function test_foo(): C::Test::T {
  return Vector {};
}
