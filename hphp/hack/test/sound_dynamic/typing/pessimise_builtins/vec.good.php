<?hh

class C {}

function test_vec(~C $lc, dynamic $d, vec<C> $v) : vec<C> {
  hh_show($v[0]);
  $v[0] = new C();
  $v[0] = $d;
  $v[] = new C();
  $v[] = $d;
  $w1 = vec<C>[$d];
  hh_show($w1);
  $w2 = vec[$d];
  hh_show($w2);
  $w3 = vec[$lc];
  hh_show($w3);
  return vec[$d];
}
