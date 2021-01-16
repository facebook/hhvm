<?hh // strict
interface A {
  <<__Rx>>
  public function f(): void;
}

interface B extends A {
  // ERROR: override immutable with mutable
  <<__Override, __Rx, __Mutable>>
  public function f(): void;
}
