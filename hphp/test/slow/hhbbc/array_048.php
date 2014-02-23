<?php

function a() { return 1; }
function foo() {
  $x = array(a());
  $y =& $x[];
  $y = 'asd';
  return $x;
}
function d() {
  $y = foo();
  var_dump($y);
}
d();
