<?hh

function test($x) {
  $a = darray[$a => $x[$a = 'foo']];
  return $a;
}

<<__EntryPoint>>
function main_1529() {
var_dump(test(darray['foo' => 5]));
}
