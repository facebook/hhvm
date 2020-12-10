<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  // ERROR: function must be reactive
  public function f(<<__MaybeMutable>>A $a): int {
    return 1;
  }
}
