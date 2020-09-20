<?hh // partial

function g(?int $x) {}

function f(mixed $x) {
  $x as ((function(int): ?int), (function(int): ?int));
  g($x);
}
