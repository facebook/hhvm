<?hh // partial

function g((int, int, string) $x) {}

function f(mixed $x) {
  if ($x is (int, int, string)) {
    g($x);
  }
}
