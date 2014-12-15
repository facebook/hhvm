<?hh // strict

abstract class A {
  public type const T = Container<int>;
}

class C {
  public type const B = D;
  public type const Test = static::B::X;

  public static function foo(): static::Test::T {
    return array();
  }
}

abstract class B {
  public type const X = A;
}

class D extends B {
}

function test_foo(): C::Test::T {
  return Vector {};
}
