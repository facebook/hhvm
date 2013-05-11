<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function foo($a, $b){
  return $a & $b;
}

function test($a, $b) {
  echo "test foo($a, $b)\n";
  for ($i = 0; $i < 20; $i++) {
    $x = foo($a, $b);
    echo $x;
    echo ", ";
  }
  echo "\n";
}

test(true, true);
test(1, 3);
test( 2.3, 4.6);
