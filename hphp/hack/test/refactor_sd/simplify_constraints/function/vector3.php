<?hh

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $v = Vector{f<>, g<>};
  ($v[0] upcast dynamic)();
}
