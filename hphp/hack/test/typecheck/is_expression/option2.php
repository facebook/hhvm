<?hh

function g(?int $x): void {}

function f(mixed $x): void {
  if ($x is ?string) {
    g($x);
  }
}
