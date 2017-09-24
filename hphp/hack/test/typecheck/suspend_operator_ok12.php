<?hh // strict

class A {
  public coroutine function f(): int {
    return 1;
  }
}

coroutine function g(): int {
  // ok - call to coroutine instance method
  $a = suspend (new A())->f();
  return $a;
}
