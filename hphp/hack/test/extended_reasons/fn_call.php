<?hh

function foo(string $_, int $_, arraykey... $_): void {}

function bar(string $x, int $y ): void{
  foo($y, $x, 1, 'a', true);
}

function qux(): string {
  return 'a';
}

function baz(int $x): void {
  $y = qux();
  bar($x, $y);
}
