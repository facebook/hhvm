<?hh // partial

function g(int $x) {}

function f(mixed $x, bool $b) {
  if ($x as @int) {
    g($x);
  }
}
