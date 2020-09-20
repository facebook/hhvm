<?hh

function foo(int $arg): void {
}

function test(): void {
  $x = foo<_>;
  $x(4);
}
