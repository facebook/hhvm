<?php

function a() { return 1; }
function foo() {
  $x = array(a());
  $y = null;
  $x[] =& $y;
  $y = 'asd';
  return $x;
}
function d() {
  $y = foo();
  var_dump($y);
  var_dump($y[0]);
}
d();
