<?hh

function rsum($s1,$s2) {
  return (int)$s1 + (int)$s2;
}
function rmul($s1,$s2) {
  return (int)$s1 * (int)$s2;
}



<<__EntryPoint>>
function main_array_reduce() {
$a = varray[1, 2, 3, 4, 5];
$b = array_reduce($a, fun("rsum"));
var_dump($b);
$c = array_reduce($a, fun("rmul"), 10);
var_dump($c);
$d = array_reduce($a, fun("rmul"));
var_dump($d);

$x = varray[];
$e = array_reduce($x, fun("rsum"), 1);
var_dump($e);
}
