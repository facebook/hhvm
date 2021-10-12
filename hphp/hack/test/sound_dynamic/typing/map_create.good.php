<?hh

function test_create_dyn_idx(dynamic $d) : Map<arraykey, int> {
  Map<arraykey, int>{$d => 1, 1 => 1};
  $x = Map{$d => 1, 1 => 1};
  hh_show($x);
  return Map{$d => 1, 1 => 1};
}
