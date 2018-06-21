<?hh // strict

class A {
  public coroutine function f(): int {
    return 1;
  }
}

class B extends A {
  // ok - override coroutine method from the base class with coroutine method
  <<__Override>>
  public coroutine function f(): int {
    return 2;
  }
}
