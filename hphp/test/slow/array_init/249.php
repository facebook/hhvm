<?hh
function foo($a) {
  $arr = varray[$a, $a++, $a];
  var_dump($arr);
}


<<__EntryPoint>>
function main_249() {
$v = 1;
foo($v);
}
