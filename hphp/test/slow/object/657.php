<?php

class A {
  var $a;
  var $b;
}

<<__EntryPoint>>
function main_657() {
;
$obj = new A();
var_dump($obj);
foreach ($obj as &$value) {
  $value = 1;
}
var_dump($obj);
$obj->c = 3;
var_dump($obj);
foreach ($obj as &$value) {
  $value = 2;
}
var_dump($obj);
}
