<?hh // strict

class A {
  // OK
  <<__Rx, __MaybeMutable>>
  public function f(): int {
    return 1;
  }
}
