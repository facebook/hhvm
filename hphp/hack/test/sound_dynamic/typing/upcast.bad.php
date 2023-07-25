<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function f(mixed $x) : void {
  $x upcast dynamic;
}

<<__SupportDynamicType>>
class MyVector<T as supportdyn<mixed>> {}

function h(MyVector<int> $x) : void {
  $x upcast MyVector<dynamic>;
  $x upcast dynamic;
}
