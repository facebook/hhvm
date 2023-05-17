<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $v = Vector{};
  $v[] = g<>;
  $w = $v;
  $w[] = f<>;
  ($v[0] upcast dynamic)(); // Report
  ($w[0] upcast dynamic)(); // Report
}
