<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  $x = f<>;
  $y = $x;
  $z = f<>;
  $x upcast dynamic;
  $y upcast dynamic;
  $z upcast dynamic;
}
