<?hh

function f1() {
  $a = Vector {
}
;
  $a->resize(4, null);
  $b = Vector {
42, 73}
;
  $a->setAll($b);
  var_dump($a);
}
function f2() {
  $a = Vector {
}
;
  $a->resize(4, null);
  $b = Map {
3 => 42, 2 => 73}
;
  $a->setAll($b);
  var_dump($a);
}
function f3() {
  $a = Map {
}
;
  $b = Vector {
42, 73}
;
  $a->setAll($b);
  var_dump($a);
}
function f4() {
  $a = Map {
}
;
  $b = Map {
3 => 42, 2 => 73}
;
  $a->setAll($b);
  var_dump($a);
}

<<__EntryPoint>>
function main_2195() {
f1();
f2();
f3();
f4();
}
