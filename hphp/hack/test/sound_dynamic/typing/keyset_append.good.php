<?hh

function test(dynamic $d, keyset<int> $ks) : void {
  $ks[] = $d;
  hh_show($ks);
}
