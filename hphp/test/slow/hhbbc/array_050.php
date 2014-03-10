<?php

function a() { return 1; }
function foo() {
  $x = array(a());
  $x[]++;
  return $x;
}
function d() {
  $y = foo();
  var_dump($y);
}
d();
