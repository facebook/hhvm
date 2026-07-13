<?hh
function foo($a) :mixed{
  $e0 = $a; $t = $a; $a++; $arr = vec[$e0, $t, $a];
  var_dump($arr);
}


<<__EntryPoint>>
function main_249() :mixed{
$v = 1;
foo($v);
}
