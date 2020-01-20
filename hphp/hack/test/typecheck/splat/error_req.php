<?hh

function f(mixed $i, int $j): void {}

function g((float, float) $t): void {
  f(...$t); // float <: #2 </: int
}
