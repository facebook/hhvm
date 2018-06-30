<?hh // strict

class A {
  // ERROR: conflicting attributes
  <<__Rx>>
  public function f(<<__MaybeMutable, __Mutable>>A $a): int {
    return 1;
  }
}
