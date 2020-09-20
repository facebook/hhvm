<?hh
function foo(int $x = 0): void {}

function f(darray<string, int> $v): void {
  foo(...$v);
}
