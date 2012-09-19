<?php

function foo($x, $y) {
  $x = 5;
  if ($x + $y) { return $x; }
  else { return 2; }
}

echo "foo(): " . foo(1, 2) . "\n";
