<?hh

function g(?int $x) {}

function f(mixed $x) {
  if ($x is ?int) {
    g($x);
  }
}
