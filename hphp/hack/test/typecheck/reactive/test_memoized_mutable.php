<?hh // strict

class Foo {
  public function __construct(public int $val) {}
}

class Test {
  // ok
  <<__Rx, __Mutable, __Memoize>>
  public function foo(int $x, int $y): void {}

  // not ok
  <<__Rx, __MutableReturn, __Memoize>>
  public function foo2(int $x, int $y): Foo {
    // UNSAFE
  }
}
