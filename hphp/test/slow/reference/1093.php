<?php

function &test(&$x) {
  $x = 1;
  return $x;
}
$x = 0;
$y = &test($x);
$y++;
var_dump($x, $y);
