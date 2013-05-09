<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.


echo "Starting\n";

function foo($a, $b, $c) {
  return $a + $b + $c;
}

function main() {
  $x = 4;
  $x = foo(1, $x == 3 ? 3: 2, $x);
  echo $x;
}

main();

echo "\ndone";
