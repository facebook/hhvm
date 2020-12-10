<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
interface A {
  <<__Rx>>
  public function f(<<__MaybeMutable>>A $a): void;
}

interface B extends A {
  // ERROR: override maybe mutable with immutable
  <<__Override, __Rx>>
  public function f(A $a): void;
}
