<?hh

function foo((string|int) $_): void {}

function bar(?string $x): void {
  foo($x);
}
