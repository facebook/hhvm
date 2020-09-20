<?hh

function foo(string $arg): void {
  echo $arg;
}

function test(): void {
  $x = foo<>;
  $x('Hello world!');
}

<<__EntryPoint>>
function main(): void {
  test();
}
