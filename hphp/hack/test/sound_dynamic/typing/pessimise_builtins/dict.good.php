<?hh

class C {}

function test_dict(vec<~C> $vlc, dynamic $d, dict<int,C> $dict, vec<~int> $vi, C $c) : dict<int,C> {
  $i = $vi[0];
  $lc = $vlc[0];
  hh_expect_equivalent<~C>($dict[0]);

  $dict[0] = new C();
  hh_expect_equivalent<dict<int, C>>($dict);
  $dict[0] = $d;
  hh_expect_equivalent<dict<int, C>>($dict);
  $dict[0] = $lc;
  hh_expect_equivalent<dict<int, C>>($dict);
  $x = $dict;
  $x[0] = $i;
  hh_expect_equivalent<dict<int, (int | C)>>($x);

  $w1 = dict<int, C>[1=>$d];
  hh_expect_equivalent<dict<int, C>>($w1);
  $w2 = dict[1=>$d];
  hh_expect_equivalent<dict<int, nothing>>($w2);
  $w3 = dict[1=>$lc];
  hh_expect_equivalent<dict<int, C>>($w3);
  $w4 = dict[1=>$c];
  hh_expect_equivalent<dict<int, C>>($w4);
  $w5 = dict[1=>$c, 2=>$i, 3=>$d];
  hh_expect_equivalent<dict<int, (int | C)>>($w5);
  return dict[1=>$d];
}
