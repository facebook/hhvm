<?hh

class C {}

<<__SupportDynamicType>>
function g(vec<int> $vi, vec<string> $vs): bool {
  return false;
}

<<__SupportDynamicType>>
function h(vec<C> $vi, vec<string> $vs): bool {
  return false;
}

function f(
  ~vec<int> $x,
  ~vec<string> $y,
  dynamic $d,
  vec<int> $z,
  vec<string> $w,
  vec<C> $c,
): ~bool {
  $r1 = g($x, $y);
  hh_expect_equivalent<~bool>($r1);
  $r2 = g($d, $y);
  hh_expect_equivalent<~bool>($r2);
  $r3 = g($z, $y);
  hh_expect_equivalent<~bool>($r3);
  $r4 = g($z, $w);
  hh_expect_equivalent<bool>($r4);
  $r5 = h($c, $w);
  hh_expect_equivalent<bool>($r5);
  return $r1;
}
