<?hh

<<__SupportDynamicType>>
function f(vec<int> $is): void {
  g(...$is);
}

<<__SupportDynamicType>>
function g(int ...$cs): void {}
