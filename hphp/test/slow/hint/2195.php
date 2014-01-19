<?php

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
  $a = StableMap {
}
;
  $b = Vector {
42, 73}
;
  $a->setAll($b);
  var_dump($a);
}
function f4() {
  $a = StableMap {
}
;
  $b = StableMap {
3 => 42, 2 => 73}
;
  $a->setAll($b);
  var_dump($a);
}
f1();
f2();
f3();
f4();
