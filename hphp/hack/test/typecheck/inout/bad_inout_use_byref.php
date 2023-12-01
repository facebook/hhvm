<?hh

function f(inout varray<string> $v): void {}

function test(): void {
  $x = vec['bar', 'baz'];
  f(inout &$x);
}
