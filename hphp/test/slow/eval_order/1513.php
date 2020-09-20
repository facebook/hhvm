<?hh
function foo($a, $b, $c) {
  var_dump($a, $b, $c);
}
function bar($a) {
  foo($a, $a++, $a);
  $arr = varray[$a, $a++, $a];
  var_dump($arr);
}


<<__EntryPoint>>
function main_1513() {
$v = 1;
bar($v);
}
