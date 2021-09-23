<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function f(int $x) : void {
  $y = $x upcast dynamic;
  hh_show($y);
}

function g<T as int>(T $x) : void {
  $y = $x upcast dynamic;
  hh_show($y);
}

function h(vec<int> $x) : void {
  $y = $x upcast vec<dynamic>;
  hh_show($y);
  $z = $x upcast dynamic;
  hh_show($z);
}

function i(Vector<dynamic> $x) : void {
  $z = $x upcast dynamic;
  hh_show($z);
}

function k() : void {
  $z = 1 upcast dynamic;
  hh_show($z);
}

function j<T as int>(T $x) : void {
  $z = ($x + 1) upcast dynamic;
  hh_show($z);
}
