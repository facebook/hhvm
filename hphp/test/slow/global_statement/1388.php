<?php
function foo() {
  $a = 5;
  global $c;
  $c = &$a;
  var_dump($c);
  return $a;
}


<<__EntryPoint>>
function main_1388() {
global $c;
$b = foo();
$b = 6;
var_dump($c);
var_dump($b);
}
