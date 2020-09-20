<?hh

function test($m1, $m2) {
  return varray[$m1 == $m2];
}

function foo($m1, $m2) {
  $a = test($m1, $m2);
  return $a[0];
}


<<__EntryPoint>>
function main_widen_bool() {
var_dump(foo(11, 11));
}
