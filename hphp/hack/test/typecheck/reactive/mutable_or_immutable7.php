<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  // OK
  <<__Rx, __MaybeMutable>>
  public function f(): int {
    return 1;
  }
}
