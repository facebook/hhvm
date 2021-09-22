<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class C<<<__NoRequireDynamic>> reify T> {}

function test(dynamic $d, C<int> $c) : void {
  $c upcast dynamic;
}
