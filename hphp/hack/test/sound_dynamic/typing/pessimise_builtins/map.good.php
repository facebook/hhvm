<?hh

class C {}

function expect_map_c(Map<int, C> $m) : void {}
function expect_map_c_or_int(Map<int, (C | int)> $m) : void {}

function test_Map(~C $lc, dynamic $d, Map<int,C> $m, ~int $i) : Map<int,C> {
  hh_show($m[0]);

  $m[0] = new C();
  $m[0] = $d;
  $m[0] = $lc;

  $w1 = Map<int,C>{0=>$d};
  expect_map_c($w1);
  $w2 = Map{0=>$d};
  expect_map_c($w2);
  $w3 = Map{0=>$lc};
  expect_map_c($w3);
  $w4 = Map{0=>new C()};
  expect_map_c($w4);
  $w5 = Map{0=>$d, 1 => $i, 2 => $lc};
  expect_map_c_or_int($w5);
  return Map{0=>$d};
}
