<?hh // strict

class A {
  public static coroutine function f(): int {
    return 1;
  }
}

class B extends A {
  public coroutine function g(): int {
    // ok - call to coroutine static method from the base class
    $a = suspend parent::f();
    return $a;
  }
}
