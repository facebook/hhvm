<?hh

function foo<T, Ta>(T $arg): void {
}

function test(): void {
  $x = foo<_>;
  $x(4);
}
