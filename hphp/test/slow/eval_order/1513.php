<?hh
function foo($a, $b, $c) :mixed{
  var_dump($a, $b, $c);
}
function bar($a) :mixed{
  $c0 = $a; $c1 = $a; $a++; $c2 = $a; foo($c0, $c1, $c2);
  $d0 = $a; $d1 = $a; $a++; $d2 = $a; $arr = vec[$d0, $d1, $d2];
  var_dump($arr);
}


<<__EntryPoint>>
function main_1513() :mixed{
$v = 1;
bar($v);
}
