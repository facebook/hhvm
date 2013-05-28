<?php

global $c;
function &foo() {
  $a = 5;
  global $c;
  $c = &$a;
  var_dump($c);
  return $a;
}
$b = foo();
$b = 6;
var_dump($c);
var_dump($b);
