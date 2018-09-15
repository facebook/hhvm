<?hh

function f(inout varray<string> $v): void {}

function test(): void {
  $x = varray['bar', 'baz'];
  f(inout &$x);
}
