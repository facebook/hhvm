<?hh // partial

function g(?int $x) {}

function f(mixed $x) {
  if ($x is ?string) {
    g($x);
  }
}
