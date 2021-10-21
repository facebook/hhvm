<?hh

<<__SupportDynamicType>>
class C {}

<<__SupportDynamicType>>
function f(vec<C> $cs): void {
  g(...$cs);
}

function g(C ...$cs): void {}
