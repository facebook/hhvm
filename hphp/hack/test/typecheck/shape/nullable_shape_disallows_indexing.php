<?hh

function foo(bool $b): void {
  $bar = ($b ? dict['a' => 5] : null)['a'];
}
