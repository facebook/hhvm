<?hh

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $v = Vector{};
  $v[] = f<>;
  $v[] = g<>;
  ($v[0] upcast dynamic)();
}
