<?hh

function test_create_dyn_idx(dynamic $d) : keyset<arraykey> {
  keyset<arraykey>[$d, 1];
  $x = keyset[$d, 1];
  hh_show($x);
  return keyset[$d, 1];
}
