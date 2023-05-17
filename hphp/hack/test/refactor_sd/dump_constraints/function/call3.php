<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $x = f<> upcast dynamic; // Don't report
  $x = f<> upcast dynamic; // Report
  $x();
  $x();
  $x = f<> upcast dynamic; // Don't report
  $x = g<> upcast dynamic; // Don't report
  $x();
}
