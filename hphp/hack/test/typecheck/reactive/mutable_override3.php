<?hh // strict
interface A {
  <<__Rx>>
  public function f(<<__Mutable>>A $a): void;
}

interface B extends A {
  // ERROR: override mutable with immutable
  <<__Override, __Rx>>
  public function f(A $a): void;
}
