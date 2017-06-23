<?php
$a = ['a' => 0, 'b' => 1, 'c' => 2, 'd' => 3];
foreach ($a as &$x) {
  foreach ($a as &$y) {
    echo "$x - $y\n";
    if ($x == 0 && $y == 1) {
      unset($a['b']);
      unset($a['c']);
    }
  }
}
