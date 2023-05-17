<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
function f(): int {
  return 1;
}

<<__SupportDynamicType>>
function g(int $i): void {}

function h(): void {
  $x = (g<> upcast dynamic);
  $y = f<>;
  $x($y());
}

function e(): void {
  $x = (g<> upcast dynamic);
  $y = (f<> upcast dynamic);
  $x($y());
}

function d(): void {
  (g<> upcast dynamic)((f<> upcast dynamic)());
}
