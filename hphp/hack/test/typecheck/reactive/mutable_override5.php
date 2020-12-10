<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
interface A {
  <<__Rx>>
  public function f(A $a): void;
}

interface B extends A {
  // ERROR: override immutable with mutable
  <<__Override, __Rx>>
  public function f(<<__Mutable>>A $a): void;
}
