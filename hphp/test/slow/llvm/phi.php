<?hh

function test_InitPackedArrayLoop($x) :mixed{
  $arr = varray[$x, $x, $x, $x, $x, $x, $x, $x, $x];
  return $arr;
}

<<__EntryPoint>>
function main_phi() :mixed{
var_dump(test_InitPackedArrayLoop(42));
}
