<?hh

class C {}

function test_Map(~C $lc, dynamic $d, Map<int,C> $v) : Map<int,C> {
  hh_show($v[0]);
  $v[0] = new C();
  $v[0] = $d;
  $w1 = Map<int,C>{0=>$d};
  hh_show($w1);
  $w2 = Map{0=>$d};
  hh_show($w2);
  $w3 = Map{0=>$lc};
  hh_show($w3);
  return Map{0=>$d};
}
