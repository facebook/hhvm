<?hh

function reverse_comp_func($n1,$n2) {
  $n1=(int)$n1; $n2=(int)$n2;
  return $n1 === $n2 ? 0 : ($n1 > $n2 ? -1 : 1);
}


<<__EntryPoint>>
function main_usort() {
$a = varray[3, 2, 5, 6, 10];
usort(inout $a, fun("reverse_comp_func"));
var_dump($a);

usort(inout $a, "undefined_function_");
}
