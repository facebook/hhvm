<?hh

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
  hh_show($arg);
  $v = ret();
  hh_show($v);
  return new :xhp();
}
