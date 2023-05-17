<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $i = 42;
  switch ($i) {
    case 0:
      $x = g<>;
      break;
    default:
      $x = f<>;
  }
  ($x upcast dynamic)();
}
