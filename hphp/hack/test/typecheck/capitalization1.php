<?hh // partial

class CamelCase {
  final public function foo(int $x): void {}
}

class child extends CAMELCASE {
  final public function foo(float $y): void {}
}

function ret(): camelcase {
  return new cAMELcASE();
}

function foo(cAmelCase $arg): :xhp {
  expect_CamelCase($arg);
  $v = ret();
  expect_CamelCase($v);
  return new :xhp();
}

function expect_CamelCase(CamelCase $x):void {}
