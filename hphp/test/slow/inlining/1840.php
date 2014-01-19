<?php

function &test(&$x,$y) {
  $GLOBALS['x'] = &$y;
  return $x[0];
}
$x = array((object)1);
$y = &test($x,0);
$y++;
var_dump($x, $y);
