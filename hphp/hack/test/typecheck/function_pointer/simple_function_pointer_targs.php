<?hh

function foo<T>(T $arg): void {
}

function test(): void {
  $x = foo<>;
  // Seems okay, since it'll get inferred I think
  $x('Hello world!');
}
