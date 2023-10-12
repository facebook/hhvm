<?hh // strict

function test(): void {
  $f = (string $x, int ...$ys) ==> {};
  $v = 123;
  $f('bar', $v, inout $v);
}
