<?php

function knuth_rand($seed) {
  // Random congruential generator from Knuth
  $n = 6364136223846793005 * $seed + 1442695040888963407;
  if ($n < 0) {
    $n = -$n;
  }
  return $n;
}

function randAccess($arr, $seed_so_far) {
  $seed_so_far = knuth_rand($seed_so_far);
  $unused_local = $arr[$seed_so_far % 22];
  return $seed_so_far;
}

$seed_so_far = 3;
$arr = array();
for ($i = 0; $i < 22; $i++) {
  $seed_so_far = knuth_rand($seed_so_far);
  $arr[$i] = $seed_so_far;
  echo " ----- "; echo $i; echo "; "; echo $arr[$i]; echo "-----\n";
}

for ($i = 0; $i < 10000000; $i++) {
  if ($i % 100000 == 0) {
    echo $seed_so_far; echo "\n";
  }
  $seed_so_far = randAccess($arr, $seed_so_far);
}
