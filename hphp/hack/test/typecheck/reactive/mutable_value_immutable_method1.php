<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx, __Mutable>>
  public function mutableMethod(): int {
    return $this->immutableMethod();
  }

  <<__Rx>>
  public function immutableMethod(): int {
    return 42;
  }
}
