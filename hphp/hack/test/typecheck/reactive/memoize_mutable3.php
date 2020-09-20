<?hh // partial

class A {
  <<__Rx, __Memoize, __Mutable>>
  public function f(): int {
    return 1;
  }
}
