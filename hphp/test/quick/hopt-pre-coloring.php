<?php
// Test the effectiveness of pre-coloring
// Shouldn't have any shuffling for concats
function foo($t0, $t1, $t2, $t3, $t4, $t5, $t6) {
  $sum = 0;
  $sum = $sum + $t0;
  $sum = $sum + $t1;
  $sum = $sum + $t2;
  $sum = $sum + $t3;
  $sum = $sum + $t4;
  $sum = $sum + $t5;
  $sum = $sum + $t6;
  $concat = $t3 . $t6;
  if ($sum > 0) {
    echo "sum = " . $sum . "\n";
    echo "concat = " . $concat . "\n";
  }
}

foo(1, 2, 3, 4, 5, 6, 7);
