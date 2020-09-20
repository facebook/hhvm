<?hh

function foo(string $arg): void {
}

function test(): void {
  $x = foo<>;
  $x('Hello world!');
}
