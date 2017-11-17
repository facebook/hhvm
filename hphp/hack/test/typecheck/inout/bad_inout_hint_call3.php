<?hh // strict

function foo((function(inout int, string): void) $f): void {}

function test(): void {
  foo(
    (int $x, $y) ==> {
    },
  );
}
