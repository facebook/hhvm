<?hh

<<__SupportDynamicType>>
function f(): void {
  $x = f<>;
  $y = $x;
  $z = f<>;
  $x upcast dynamic;
  $y upcast dynamic;
  $z upcast dynamic;
}
