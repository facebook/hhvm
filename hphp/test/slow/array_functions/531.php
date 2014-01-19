<?php

function f($x, $y) {
  var_dump($x, $y);
  return $x + $x + $y + 1;
}
var_dump(array_reduce(array(), 'f'));
var_dump(array_reduce(array(), 'f', null));
var_dump(array_reduce(array(), 'f', 0));
var_dump(array_reduce(array(), 'f', 23));
var_dump(array_reduce(array(4), 'f'));
var_dump(array_reduce(array(4), 'f', null));
var_dump(array_reduce(array(4), 'f', 0));
var_dump(array_reduce(array(4), 'f', 23));
var_dump(array_reduce(array(1,2,3,4), 'f'));
var_dump(array_reduce(array(1,2,3,4), 'f', null));
var_dump(array_reduce(array(1,2,3,4), 'f', 0));
var_dump(array_reduce(array(1,2,3,4), 'f', 23));

