<?hh

function foo<T>(T $arg): void {
}

function test(): void {
  $x = foo<int, int>;
  $x(4);
}
