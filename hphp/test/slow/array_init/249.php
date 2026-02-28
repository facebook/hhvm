<?hh
function foo($a) :mixed{
  $arr = vec[$a, $a++, $a];
  var_dump($arr);
}


<<__EntryPoint>>
function main_249() :mixed{
$v = 1;
foo($v);
}
