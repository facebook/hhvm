<?hh // partial

function g(int $x) {}

function f(mixed $x) {
  $x as int;
  g($x);
}
