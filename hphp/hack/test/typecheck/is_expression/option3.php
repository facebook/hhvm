<?hh // partial

function g((int, int) $x) {}

function f((int, int) $x) {
  if ($x is ?(num, arraykey)) {
    g($x);
  }
}
