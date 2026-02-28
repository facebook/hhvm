<?hh

function foo((function(inout int): void) $f): void {}

function test(): void {
  foo(
    $v ==> {
    },
  );
}
