<?php

function &add1(&$x) {
  $x += 1;
  return $x;
}

$y = 10;
add1($y);
var_dump($y);
var_dump(add1(add1($y)));
