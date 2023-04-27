<?hh

<<__SupportDynamicType>>
function f(): void {}

<<__SupportDynamicType>>
function g(): void {}

function h(): void {
  $v = vec[f<>, g<>];
  ($v[0] upcast dynamic)();
}

function e(): void {
  $w = vec[g<>];
  $w[] = f<>;
  ($w[0] upcast dynamic)();
}

function d(): void {
  $w = vec[f<>];
  $w[] = g<>;
  ($w[0] upcast dynamic)();
}
