<?hh

function foo(int $x, string $y): void {}

function bar(): int {
  $x = 109574;
  $y = "my_string";
  foo($x, $y);
  foo($x, $y);
  foo($x, $y);
  foo($x, $y);
  foo($x, $y);
  return $x;
}
