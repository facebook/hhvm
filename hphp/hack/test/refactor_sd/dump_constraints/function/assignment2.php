<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function g(): void {}

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  $b = f<>;
  $b = g<>;
  $b upcast dynamic;

  $c = g<>;
  $c = f<>;
  $c upcast dynamic;
}
