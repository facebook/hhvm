<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  $b = f<>;
  $index = 1;
  while ($index < 10) {
    if ($index == 9) {
      $b upcast dynamic;
    }
    $index++;
  }
}
