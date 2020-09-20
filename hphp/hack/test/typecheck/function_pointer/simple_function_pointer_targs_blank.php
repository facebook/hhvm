<?hh

function foo<T, Ta>(T $arg): void {
}

function test(): void {
  $x = foo<int, _>;
  $x(4);
}
