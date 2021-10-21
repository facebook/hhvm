<?hh

class C {}

function test_dict(~C $lc, dynamic $d, dict<int,C> $dict, ~int $i) : dict<int,C> {
  hh_show($dict[0]);

  $dict[0] = new C();
  hh_show($dict);
  $dict[0] = $d;
  hh_show($dict);
  $dict[0] = $lc;
  hh_show($dict);
  $x = $dict;
  $x[0] = $i;
  hh_show($x);

  $w1 = dict<int, C>[1=>$d];
  hh_show($w1);
  $w2 = dict[1=>$d];
  hh_show($w2);
  $w3 = dict[1=>$lc];
  hh_show($w3);
  $w4 = dict[1=>new C()];
  hh_show($w4);
  $w5 = dict[1=>new C(), 2=>$i, 3=>$d];
  hh_show($w5);
  return dict[1=>$d];
}
