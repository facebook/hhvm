<?hh

function test_dict_key(vec<~float> $vfi, keyset<int> $ks) : void {
  $fi = $vfi[0];
  $ks[$fi];
  $ks[] = $fi;
  $ks[$fi];
}
