<?hh //strict

function foo(): void {
  $bar = (true ? array('a' => 5) : null)['a'];
}
