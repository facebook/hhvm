<?hh // partial

function g(int $x) {}

function f(mixed $x, bool $b) {
  $x as (function(int): ?int);
  g($x);
}
