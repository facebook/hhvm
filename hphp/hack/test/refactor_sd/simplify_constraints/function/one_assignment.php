<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  $x = f<>;
  ($x upcast dynamic)();
}
