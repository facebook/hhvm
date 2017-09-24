<?hh // strict

class A {
  public static coroutine function f(): int {
    return 1;
  }

  public coroutine function b(): int {
    // ok - call to coroutine static method
    $a = suspend static::f();
    return $a;
  }
}
