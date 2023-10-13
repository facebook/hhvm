<?hh

function f(inout string $s): void {
  $s = 'z';
}

function test(): void {
  $x = 'foobar';
  f(inout $x[5]);
}
