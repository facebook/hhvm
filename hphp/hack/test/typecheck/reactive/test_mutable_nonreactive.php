<?hh // strict

class Test {

  public function __construct(public int $val) {}
}

function foo(<<__Mutable>>Test $x, Test $y): void {}
