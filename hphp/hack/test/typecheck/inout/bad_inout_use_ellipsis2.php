<?hh

function test(): void {
  $f = function(int $x, ...) {};
  $v = 123;
  $f(42, $v, inout $v);
}
