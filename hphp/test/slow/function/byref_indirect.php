<?php

$x = 1;

function &byref() {
  return $GLOBALS['x'];
}

$byref = 'byref';
$m =& $byref();
$m = 2;
echo $x;
echo "\n";
