<?php

function foo($x, $y) {
  if ($x + $y) { return $x; }
  else { return 2; }
}

echo "foo(): " . foo(1, 2) . "\n";
