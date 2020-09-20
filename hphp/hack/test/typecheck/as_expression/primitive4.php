<?hh // partial

function g(int $x) {}

function f(mixed $x, bool $b) {
  if ($b) {
    if ($x as int) {
      g($x);
    }
  }
  g($x);
}
