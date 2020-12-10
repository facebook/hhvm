<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  public function nonrx(): int {
    return 42;
  }
  <<__RxLocal, __Mutable>>
  public function rxlocal(): int {
    return $this->nonrx();
  }
}
