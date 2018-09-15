<?hh // strict

class A {
  public coroutine function number(): int {
    return 1;
  }
}

class B extends A {
  <<__Override>>
  public function number(): int {
    // not ok - override of coroutine method with regular one
    return 2;
  }
}
