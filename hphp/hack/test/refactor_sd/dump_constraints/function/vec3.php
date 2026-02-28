<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $v = vec[f<>, g<>];
  $v[0] upcast dynamic;

  $w = vec[g<>];
  $w[] = f<>;
  $w[0] upcast dynamic;
}
