<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function f(mixed $x) : void {
  $x upcast dynamic;
}

function h(Vector<int> $x) : void {
  $x upcast Vector<dynamic>;
  $x upcast dynamic;
}
