<?hh

function g(int $x): void {}

function f(mixed $x, bool $b): void {
  if ($x is (function(int): ?int)) {
    g($x);
  }
}
