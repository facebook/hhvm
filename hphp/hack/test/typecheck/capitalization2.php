//// f1.php
<?hh

class CamelCase {
  final public function foo(int $x): void {}
}

//// f2.php
<?hh
function foo(cAmelCase $arg): :xhp {
  hh_show($arg);
  $v = ret();
  hh_show($v);
  return new :xhp();
}

function ret(): camelcase {
  return new cAMELcASE();
}
