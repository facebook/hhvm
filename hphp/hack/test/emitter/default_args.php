<?hh // strict

function foo(int $x, string $lol = ''): void {
  var_dump($lol);
}
function f(int $x): int { return 12+$x; }
function bar(int $x, string $lol = (string)f($x)): void {
  var_dump($lol);
}
function baz(int $x, string $lol = (string)f($x), int $lurr = 50): void {
  var_dump($lol);
  var_dump($lurr);
}

function test(): void {
  foo(10);
  foo(10, 'hello');
  bar(10);
  bar(10, 'hello');
  baz(10);
  baz(10, 'hello');
  baz(10, 'hello', 1);
}
