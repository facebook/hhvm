<?hh

function g(?int $x): void {}

function f(mixed $x): void {
  if ($x is ((function(int): ?int), (function(int): ?int))) {
    g($x);
  }
}
