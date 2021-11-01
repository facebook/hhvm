<?hh

function g(int $x): void {}

function f(mixed $x): void {
  $y = $x ?as int;
  g($y);
}
