<?hh

function g((int, int, string) $x): void {}

function f(mixed $x): void {
  $x as (int, string, string);
  g($x);
}
