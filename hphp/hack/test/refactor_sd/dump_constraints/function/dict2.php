<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $d = dict['f' => f<>, 'g' => g<>];
  $d['f'] upcast dynamic;
}
