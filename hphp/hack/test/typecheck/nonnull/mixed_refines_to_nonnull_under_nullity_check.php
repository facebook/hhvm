<?hh

function f(nonnull $x): void {}

function g(mixed $x): void {
  if ($x !== null) {
    f($x);
  }
}
