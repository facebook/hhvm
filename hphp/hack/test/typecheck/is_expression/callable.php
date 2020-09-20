<?hh // partial

function g(int $x) {}

function f(mixed $x, bool $b) {
  if ($x is (function(int): ?int)) {
    g($x);
  }
}
