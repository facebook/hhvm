<?hh

<<__SupportDynamicType>> class C {}

function test_vec(~C $lc, dynamic $d, vec<C> $v, ~int $i) : vec<C> {
  hh_show($v[0]);

  $v[0] = new C();
  hh_show($v);
  $v[0] = $d;
  hh_show($v);
  $v[0] = $lc;
  hh_show($v);
  $w = $v;
  $w[0] = $i;
  hh_show($w);


  $v[] = new C();
  hh_show($v);
  $v[] = $d;
  hh_show($v);
  $v[] = $lc;
  hh_show($v);
  $w = $v;
  $w[] = $i;
  hh_show($w);

  $w1 = vec<C>[$d];
  hh_show($w1);
  $w2 = vec[$d];
  hh_show($w2);
  $w3 = vec[$lc];
  hh_show($w3);
  $w4 = vec[new C()];
  hh_show($w4);
  $w5 = vec[new C(), $i, $d];
  hh_show($w5);
  return vec[$d];
}
