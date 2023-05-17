<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $v = vec[];
  $v[] = g<>;
  $w = $v;
  $v[] = f<>;
  $v[0] upcast dynamic; // Report
  $w[0] upcast dynamic; // Don't report
}
