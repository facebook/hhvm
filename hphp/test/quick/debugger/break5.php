<?php

// Warning: line numbers are sensitive, do not change

function bad() {
  global $x;
  $x += 10;
  return true;
}

$x = 1;
echo $x."\n";
echo $x."\n";
$x = 1;
