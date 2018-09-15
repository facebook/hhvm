<?hh

function f(inout $x) {
  return ++$x;
}


<<__EntryPoint>>
function main_double_array_get() {
$a = array(0, 1, 2);
$b = array(1, 2, 3);
var_dump(f(inout $b[$a[1]]));
}
