<?hh

function test_InitPackedArrayLoop($x) {
  $arr = varray[$x, $x, $x, $x, $x, $x, $x, $x, $x];
  return $arr;
}

<<__EntryPoint>>
function main_phi() {
var_dump(test_InitPackedArrayLoop(42));
}
