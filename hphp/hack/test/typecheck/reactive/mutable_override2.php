<?hh // strict
interface A {
  <<__Rx, __Mutable>>
  public function f(): void;
}

interface B extends A {
  // ERROR: override mutable with immutable
  <<__Override, __Rx>>
  public function f(): void;
}
