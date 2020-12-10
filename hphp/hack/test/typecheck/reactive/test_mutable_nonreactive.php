<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class Test {

  public function __construct(public int $val) {}
}

function foo(<<__Mutable>>Test $x, Test $y): void {}
