<?hh

function f(int ...$i): void {}

function g(vec<float> $v): void {
  f(...$v); // float <: #1 </: int
}
