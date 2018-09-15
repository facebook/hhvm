<?hh // strict

function foo((function(int): void) $f): void {}

function test(): void {
  $x = (inout $v) ==> {
  };
  foo($x);
}
