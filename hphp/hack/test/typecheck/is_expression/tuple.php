<?hh

function g((int, int, string) $x): void {}

function f(mixed $x): void {
  if ($x is (int, int, string)) {
    g($x);
  }
}
