<?php

// Lazy infinite list!
function fibonacci($first, $second) {
  $a = $first; $b = $second;
  while (true) {
    yield $b;
    $temp = $b;
    $b = $a + $b;
    $a = $temp;
  }
}

foreach (fibonacci(0, 1) as $k) {
  if ($k > 1000) break;
  echo $k . "\n";
}
