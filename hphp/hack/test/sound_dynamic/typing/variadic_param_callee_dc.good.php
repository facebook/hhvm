<?hh

<<__DynamicallyCallable>>
function f(vec<int> $is): void {
  g(...$is);
}

<<__DynamicallyCallable>>
function g(int ...$cs): void {}
