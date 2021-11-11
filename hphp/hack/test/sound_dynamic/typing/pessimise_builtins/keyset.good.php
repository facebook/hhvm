<?hh

function test_keyset(vec<~int> $vli, vec<~string> $vls, dynamic $d, keyset<int> $ks) : void {
  $li = $vli[0];
  $ls = $vls[0];
  hh_expect_equivalent<~int>($ks[$li]);
  hh_expect_equivalent<~int>($ks[$ls]);
  hh_expect_equivalent<~int>($ks[$d]);
  hh_expect_equivalent<~int>($ks[1]);

  $ks[] = $li;
  hh_expect_equivalent<keyset<arraykey>>($ks);
  $ks[] = $ls;
  hh_expect_equivalent<keyset<arraykey>>($ks);

  $a = keyset[$d];
  hh_expect_equivalent<keyset<arraykey>>($a);
  $b = keyset[$li];
  hh_expect_equivalent<keyset<arraykey>>($b);
  $c = keyset[$ls];
  hh_expect_equivalent<keyset<arraykey>>($c);
  $d = keyset[$ls, $li];
  hh_expect_equivalent<keyset<arraykey>>($d);
}
