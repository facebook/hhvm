<?hh

function test_create_dyn_idx(dynamic $d) : Map<(~int & arraykey), int> {
  Map<arraykey, int>{$d => 1, 1 => 1};
  $x = Map{$d => 1, 1 => 1};
  hh_expect_equivalent<Map<(~int & arraykey), int>>($x);
  return Map{$d => 1, 1 => 1};
}
