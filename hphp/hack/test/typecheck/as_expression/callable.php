<?hh

function g(int $x): void {}

function f(mixed $x, bool $b): void {
  $x as (function(int): ?int);
  g($x);
}
