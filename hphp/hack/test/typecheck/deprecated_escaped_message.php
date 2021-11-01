<?hh

<<__Deprecated('testing \n \t \\ \''.' foo')>>
function foo(): void {}

function f(): void {
  foo();
}
