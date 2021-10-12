<?hh

class C {}

function test_dict(~C $lc, dynamic $d, dict<int,C> $v) : dict<int,C> {
  hh_show($v[0]);
  $v[0] = new C();
  $v[0] = $d;
  $w1 = dict<int, C>[1=>$d];
  hh_show($w1);
  $w2 = dict[1=>$d];
  hh_show($w2);
  $w3 = dict[1=>$lc];
  hh_show($w3);
  return dict[1=>$d];
}
