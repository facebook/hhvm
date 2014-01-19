<?php

function f($x, &$y) {
  $x = "x";
  $y = "y";
}
$a = array(0, 1);
f($a[0], $a[1]);
var_dump($a);
