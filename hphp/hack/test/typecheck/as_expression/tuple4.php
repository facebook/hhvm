<?hh

function g(?int $x): void {}

function f(mixed $x): void {
  $x as ((function(int): ?int), (function(int): ?int));
  g($x);
}
