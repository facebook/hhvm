<?php

function return_true() {
 return true;
 }
function f(&$x, $y) {
  $x = $y;
  if (return_true())
    unset($x);
  $x = 0;
}
$myvar = 10;
f($myvar, 30);
var_dump($myvar);
