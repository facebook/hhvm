<?hh // strict

class A {
  // ERROR: conflicting attributes
  <<__Rx, __Mutable, __MaybeMutable>>
  public function f(): int {
    return 1;
  }
}
