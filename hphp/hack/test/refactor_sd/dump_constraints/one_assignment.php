<?hh

<<__SupportDynamicType>>
function f(): void {
  $x = f<>;
  $x upcast dynamic;
}
