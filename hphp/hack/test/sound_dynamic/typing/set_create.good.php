<?hh

function test_create_dyn_idx(dynamic $d) : Set<(~int & arraykey)> {
  Set<arraykey>{$d, 1};
  $x = Set{$d, 1};
  hh_expect_equivalent<Set<(~int & arraykey)>>($x);
  return Set{$d, 1};
}
