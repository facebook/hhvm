<?hh

function f1() :mixed{
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
function f2() :mixed{
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
function f3() :mixed{
  $a = Map {
}
;
  $b = Vector {
42, 73}
;
  $a->setAll($b);
  var_dump($a);
}
function f4() :mixed{
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
function main_2195() :mixed{
f1();
f2();
f3();
f4();
}
