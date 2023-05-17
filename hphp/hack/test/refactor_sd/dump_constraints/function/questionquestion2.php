<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $x = null;
  $x ??= f<>;
  $x upcast dynamic;
}
