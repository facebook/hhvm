<?php

function foo(&$x, $y) {
  $x = array(1,2);
  $y = $x;
  return $y;
}

$x = 0;
$y = 0;
var_dump(foo($x, $y));
var_dump($x);
