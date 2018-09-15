<?hh //strict

function foo(bool $b): void {
  $bar = ($b ? array('a' => 5) : null)['a'];
}
