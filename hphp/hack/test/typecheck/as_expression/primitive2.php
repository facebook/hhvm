<?hh // partial

function g(int $x) {}

function f(mixed $x) {
  $y = $x ?as int;
  g($y);
}
