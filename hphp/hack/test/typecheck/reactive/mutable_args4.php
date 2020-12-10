<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx, __Mutable>>
  public function f(): void {
    // ERROR: cannot borrow mutable value more than once
    $this->g($this);
  }
  <<__Rx, __Mutable>>
  public function g(<<__Mutable>>A $a): void {
  }
}
