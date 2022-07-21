<?hh

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
  !((f<> upcast dynamic)() is nonnull);
}
