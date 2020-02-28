<?hh

function test($x) {
  apc_store('foo', varray['a'.$x, varray[$x]]);
  $a = __hhvm_intrinsics\apc_fetch_no_check('foo');
  $x = array_intersect($a, $a);
  var_dump($x);
}


// disable array -> "Array" conversion notice
<<__EntryPoint>>
function main_1826() {
error_reporting(error_reporting() & ~E_NOTICE);
test('foo');
}
