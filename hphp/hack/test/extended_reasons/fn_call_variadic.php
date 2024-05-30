<?hh

function foo(arraykey... $_): void {}

function bar(int $x, string $y, bool $z): void{
  foo($x, $y, $z);
}
