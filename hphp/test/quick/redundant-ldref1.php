<?php

function foo(&$x) {
  return $x + $x;
}

$x = 1;
var_dump(foo($x));
var_dump($x);
