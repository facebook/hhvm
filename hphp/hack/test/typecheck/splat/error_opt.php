<?hh

function f(mixed $i, int $j = 3): void {}

function g((float, float) $t): void {
  f(...$t); // float <: #2 </: int
}
