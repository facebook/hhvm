<?hh

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $v = vec[];
  $v[] = f<>;
  $v[] = g<>;
  $v[0] upcast dynamic;
}
