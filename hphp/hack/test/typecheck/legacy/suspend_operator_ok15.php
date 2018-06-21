<?hh // strict

class A {
  public coroutine function f(): int {
    return 1;
  }
}

class B extends A {
  public coroutine function g(): int {
    // ok - call to coroutine instance method from the base class
    $a = suspend parent::f();
    return $a;
  }
}
