<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function f(int $x) : void {
  $y = $x upcast dynamic;
  hh_expect_equivalent<dynamic>($y);
}

function g<T as int>(T $x) : void {
  $y = $x upcast dynamic;
  hh_expect_equivalent<dynamic>($y);
}

function h(vec<int> $x) : void {
  $y = $x upcast vec<dynamic>;
  hh_expect_equivalent<vec<dynamic>>($y);
  $z = $x upcast dynamic;
  hh_expect_equivalent<dynamic>($z);
}

function k() : void {
  $z = 1 upcast dynamic;
  hh_expect_equivalent<dynamic>($z);
}

function j<T as int>(T $x) : void {
  $z = ($x + 1) upcast dynamic;
  hh_expect_equivalent<dynamic>($z);
}

class C {
  public dynamic $d = 3 upcast dynamic;
}
