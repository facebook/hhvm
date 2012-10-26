<?php

function f1($x) {
  return true < $x;
}
var_dump(f1(4));

function f2($x) {
  return "0.0" == $x;
}
var_dump(f2(false));

function f3($x) {
  $y = array(1);
  return $x == $y;
}
var_dump(f3(3));

