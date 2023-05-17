<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $d = dict['f' => f<>];
  $d['g'] = g<>;
  ($d['f'] upcast dynamic)();

  $d = dict['g' => g<>];
  $d['f'] = f<>;
  ($d['f'] upcast dynamic)();
}
