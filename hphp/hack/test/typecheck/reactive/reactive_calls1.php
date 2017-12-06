<?hh // strict
<<__Rx>>
function foo(): void {}

function baz(): void {}
function bar(): void {
  // bar can call baz
  baz();
  // bar can call foo
  foo();
}

<<__Rx>>
function qux(): void {
  // qux can call foo
  foo();
  // qux cannot call baz
  baz();
}
