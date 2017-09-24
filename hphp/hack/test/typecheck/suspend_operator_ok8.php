<?hh // strict

class A {
  public static coroutine function f(): int {
    return 1;
  }
}

coroutine function g(): int {
  // ok - call to coroutine static method
  $a = suspend A::f();
  return $a;
}
