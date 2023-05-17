<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function g(): void {}

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  if (true) {
    f<> upcast dynamic;
  } else {
    g<> upcast dynamic;
  }

  if (true) {
    $b = g<>;
  } else {
    $b = f<>;
  }
  $b upcast dynamic;
}
