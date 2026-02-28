<?hh

function foo((function(inout int): void) $f): void {}

function test(): void {
  $z = 42;
  $x = function(int $v) use ($z) {};
  foo($x);
}
