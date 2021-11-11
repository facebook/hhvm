<?hh

<<__SupportDynamicType>> class C {}

function test_vec(vec<~C> $vlc, dynamic $d, vec<C> $v, vec<~int> $vi, C $c) : vec<C> {
  $lc = $vlc[0];
  $i = $vi[0];

  hh_expect_equivalent<~C>($v[0]);

  $v[0] = new C();
  hh_expect_equivalent<vec<C>>($v);
  $v[0] = $d;
  hh_expect_equivalent<vec<C>>($v);
  $v[0] = $lc;
  hh_expect_equivalent<vec<C>>($v);
  $w = $v;
  $w[0] = $i;
  hh_expect_equivalent<vec<(int | C)>>($w);


  $v[] = new C();
  hh_expect_equivalent<vec<C>>($v);
  $v[] = $d;
  hh_expect_equivalent<vec<C>>($v);
  $v[] = $lc;
  hh_expect_equivalent<vec<C>>($v);
  $w = $v;
  $w[] = $i;
  hh_expect_equivalent<vec<(int | C)>>($w);

  $w1 = vec<C>[$d];
  hh_expect_equivalent<vec<C>>($w1);
  $w2 = vec[$d];
  hh_expect_equivalent<vec<nothing>>($w2);
  $w3 = vec[$lc];
  hh_expect_equivalent<vec<C>>($w3);
  $w4 = vec[$c];
  hh_expect_equivalent<vec<C>>($w4);
  $w5 = vec[$c, $i, $d];
  hh_expect_equivalent<vec<(int | C)>>($w5);
  return vec[$d];
}
