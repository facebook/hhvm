<?hh

function test_map_key(vec<~int> $vli, vec<~string> $vls, dynamic $d, Map<int, int> $m) : void {
  $li = $vli[0];
  $ls = $vls[0];
  hh_expect_equivalent<~int>($m[$li]);
  hh_expect_equivalent<~int>($m[$d]);
  hh_expect_equivalent<~int>($m[1]);
  $m[$li] = 1;
  hh_expect_equivalent<Map<int, int>>($m);
  $m[$d] = 1;
  hh_expect_equivalent<Map<int, int>>($m);

  $a = Map{$d=>1};
  hh_expect_equivalent<Map<arraykey, int>>($a);
  $b = Map{$li=>1};
  hh_expect_equivalent<Map<arraykey, int>>($b);
  $c = Map{$ls=>1};
  hh_expect_equivalent<Map<arraykey, int>>($c);
  $d = Map{$ls=>1, $li=>1};
  hh_expect_equivalent<Map<arraykey, int>>($d);
}
