<?php

function f1() {
  $x = array(1,2,3);
  unset($x[0]);
  var_dump($x);
}
function f2() {
  $x = array(1,2,3);
  unset($x[0][0]);
  var_dump($x);
}
function f3() {
  $x = array(array(4,5,6),2,3);
  unset($x[0][0]);
  var_dump($x);
}
function f4() {
  $x = array(array(4,5,6),2,3);
  unset($x[0][0][0]);
  var_dump($x);
}
f1();
f2();
f3();
f4();
