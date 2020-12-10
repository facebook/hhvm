<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  // ERROR: method must be reactive
  <<__MaybeMutable>>
  public function f(): int {
    return 1;
  }
}
