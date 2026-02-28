<?hh

function test(dynamic $d, keyset<int> $ks) : void {
  $ks[] = $d;
  hh_expect_equivalent<keyset<(~int & arraykey)>>($ks);
}
