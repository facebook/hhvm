<?hh
function foo($a, $b, $c) :mixed{
  var_dump($a, $b, $c);
}
function bar($a) :mixed{
  foo($a, $a++, $a);
  $arr = vec[$a, $a++, $a];
  var_dump($arr);
}


<<__EntryPoint>>
function main_1513() :mixed{
$v = 1;
bar($v);
}
