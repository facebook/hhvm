<?php
function foo($a, $b) {
  // const * const
  $a = 5;
  $b = -7;
  return $a * $b;
}

echo foo(5, -7);
echo "\n";
