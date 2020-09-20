<?hh // partial

function g(int $x) {}

function f(mixed $x) {
  if ($x as int) {
    g($x);
  }
  g($x);
}
