<?hh

function id<T>(T $x): T {
  return $x;
}

function extend(&$dst, $src) {}

function test(): void {
  $x = vec[];
  id($x);
  extend(&$x, vec[42]);
}
