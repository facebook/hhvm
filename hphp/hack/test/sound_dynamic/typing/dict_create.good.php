<?hh

function test_create_dyn_idx(dynamic $d) : dict<arraykey, int> {
  dict<arraykey, int>[$d => 1, 1 => 1];
  $x = dict[$d => 1, 1 => 1];
  hh_expect_equivalent<dict<arraykey, int>>($x);
  return dict[$d => 1, 1 => 1];
}
