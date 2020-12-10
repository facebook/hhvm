<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class Test {
  public function __construct(public int $val) {}
}
class Another {
  <<__Mutable>>
  public function mutableNonReactive(Test $x, Test $y): void {}
}
