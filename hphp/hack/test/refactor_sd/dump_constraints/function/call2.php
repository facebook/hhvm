<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  $w = f<> upcast dynamic; // Don't report
  $x = f<> upcast dynamic; // Report
  $x();
  $y = f<> upcast dynamic; // Don't report
}
