<?php
function foo($a, $b) {
  // reg * const
  $b = -7;
  return $a * $b;
}

echo foo(5, -7);
echo "\n";
