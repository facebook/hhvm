<?hh

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $x = null;
  $x ??= f<>;
  $x upcast dynamic;
}
