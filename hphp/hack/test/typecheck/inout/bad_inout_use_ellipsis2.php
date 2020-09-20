<?hh // partial

function test(): void {
  $f = function(int $x, ...$_) {};
  $v = 123;
  $f(42, $v, inout $v);
}
