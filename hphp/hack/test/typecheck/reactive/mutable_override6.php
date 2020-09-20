<?hh // strict
interface A {
  <<__Rx, __MaybeMutable>>
  public function f(): void;
}

interface B extends A {
  // ERROR: override maybe mutable with immutable
  <<__Override, __Rx>>
  public function f(): void;
}
