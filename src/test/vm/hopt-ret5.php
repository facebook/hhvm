<?php

function &foo(&$x) {
  return $x;
}

$x = 34;
$y =& foo($x);
$y++;

var_dump($x);
var_dump($y);
