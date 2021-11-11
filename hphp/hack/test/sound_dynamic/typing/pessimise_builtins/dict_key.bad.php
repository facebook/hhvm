<?hh

function test_dict_key(vec<~float> $vfi, dict<int, int> $di) : void {
  $fi = $vfi[0];
  $di[$fi];
  $di[$fi] = 1;
  dict[$fi => 1];
}
