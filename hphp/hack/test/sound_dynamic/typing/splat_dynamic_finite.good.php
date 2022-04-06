<?hh

function g(~int $i, ~float $d): void {}

function f(dynamic $d): void {
  g(...$d);
}
