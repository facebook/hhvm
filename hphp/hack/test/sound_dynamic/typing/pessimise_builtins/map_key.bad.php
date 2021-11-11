<?hh

function test_map_key(vec<~float> $vfi, vec<~string> $vls, Map<int, int> $m) : void {
  $fi = $vfi[0];
  $ls = $vls[0];
  $m[$fi];
  $m[$ls];
  $m[$fi] = 1;
  $m[$ls] = 1;
  Map {$fi => 1};
}
