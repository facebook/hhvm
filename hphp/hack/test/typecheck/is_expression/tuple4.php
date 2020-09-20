<?hh // partial

function g(?int $x) {}

function f(mixed $x) {
  if ($x is ((function(int): ?int), (function(int): ?int))) {
    g($x);
  }
}
