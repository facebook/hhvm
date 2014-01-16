<?php

function foo($arr, $n, ...) {
  $fga = array_slice(func_get_args(), 2);
  $fgb = array_slice(func_get_args(), $n);
  var_dump($fga === $fgb);
  return $fga;
}

function foo2(...) {
  return array_slice(func_get_args(), 1);
}

var_dump(foo(array(2, 3), 2, array(2)) === array(array(2)));
var_dump(foo(array(2, 3), 2, array(1), array(2)) === array(array(1), array(2)));
var_dump(foo(array(2, 3), 2) === array());
var_dump(foo2() == array());
