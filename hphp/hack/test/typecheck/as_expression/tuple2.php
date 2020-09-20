<?hh // partial

function g((int, int, string) $x) {}

function f(mixed $x) {
  $x as (int, string, string);
  g($x);
}
