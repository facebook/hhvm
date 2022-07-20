<?hh

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $b = true;
  ($b ? f<> : g<>) upcast dynamic;
}
