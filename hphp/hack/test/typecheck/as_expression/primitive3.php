<?hh

function g(int $x): void {}

function f(mixed $x): void {
  if ($x as int) {
    g($x);
  }
  g($x);
}
