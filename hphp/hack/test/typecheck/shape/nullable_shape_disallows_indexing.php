<?hh //strict

function foo(bool $b): void {
  $bar = ($b ? darray['a' => 5] : null)['a'];
}
