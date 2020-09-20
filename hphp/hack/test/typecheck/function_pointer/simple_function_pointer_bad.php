<?hh

function foo(string $arg): void {
}

function test(): void {
  $x = foo<>;
  // Whoops, expects a string
  $x(4);
}
