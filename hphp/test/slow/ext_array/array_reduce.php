<?hh

function rsum($s1,$s2) :mixed{
  return (int)$s1 + (int)$s2;
}
function rmul($s1,$s2) :mixed{
  return (int)$s1 * (int)$s2;
}



<<__EntryPoint>>
function main_array_reduce() :mixed{
$a = vec[1, 2, 3, 4, 5];
$b = array_reduce($a, rsum<>);
var_dump($b);
$c = array_reduce($a, rmul<>, 10);
var_dump($c);
$d = array_reduce($a, rmul<>);
var_dump($d);

$x = vec[];
$e = array_reduce($x, rsum<>, 1);
var_dump($e);
}
