<?hh

function g((int, int) $x): void {}

function f((int, int) $x): void {
  if ($x is ?(num, arraykey)) {
    g($x);
  }
}
