<?hh

function g(?int $x): void {}

function f(mixed $x): void {
  $x as ?string;
  g($x);
}
