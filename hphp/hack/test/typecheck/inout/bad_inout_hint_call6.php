<?hh // strict

function foo((function(inout int): void) $f): void {}

function test(): void {
  foo(
    (inout num $v) ==> {
    },
  );
}
