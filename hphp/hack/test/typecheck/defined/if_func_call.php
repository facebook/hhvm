<?hh // strict

function foo(int $i): void {}

function bar(bool $b): int {
  if ($b) {
    foo($x = 1);
  } else {
    foo($x = 2);
  }
  return $x;
}
