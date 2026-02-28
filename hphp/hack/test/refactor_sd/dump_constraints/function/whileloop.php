<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  $index = 1;
  while ($index < 10) {
    f<> upcast dynamic;
    $index++;
  }
}
