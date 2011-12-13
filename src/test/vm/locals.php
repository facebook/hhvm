<?php
function f($a, $b, $c) {
  $x = $a + $b;
  $y = $a - $b;
  if ($x <= 0 && $x >= 0) $x = 1;
  if ($y <= 0 && $y >= 0) $y = 1;
  $z = $c - 1;
  if ($z <= 0) {
    return $x * $y;
  }
  return f($x+0, $y+0, $z+0);
}
echo f(12,7,5);
echo "\n";
