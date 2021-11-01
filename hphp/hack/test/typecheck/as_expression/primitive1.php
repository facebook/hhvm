<?hh

function g(int $x): void {}

function f(mixed $x): void {
  $x as int;
  g($x);
}
