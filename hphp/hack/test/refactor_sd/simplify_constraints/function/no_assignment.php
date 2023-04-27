<?hh

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $x = g<>;
  (g<> upcast dynamic)();
  ($x upcast dynamic)();
}
