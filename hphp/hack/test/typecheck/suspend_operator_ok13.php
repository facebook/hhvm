<?hh // strict

class A {
  public coroutine function f(): int {
    return 1;
  }
  public coroutine function g(): int {
    // ok - call to coroutine instance method
    $a = suspend $this->f();
    return $a;
  }
}
