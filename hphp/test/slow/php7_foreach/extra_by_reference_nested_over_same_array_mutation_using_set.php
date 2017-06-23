<?php
$a = [0, 1];
foreach ($a as &$x) {
  foreach ($a as &$y) {
    echo "$x - $y\n";
    $a[3] = 3;
  }
}
