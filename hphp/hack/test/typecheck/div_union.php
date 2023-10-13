<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function inverseCDF(float $p, int $n): float {
  $v = 0.5;
  $dv = 0.5;
  $x = 0.0;
  while ($dv > 1e-15) {
    $x = 1 / $v - 1;
    $dv = $dv / 2;
    if (cdf($x, $n) < $p) {
      $v = $v - $dv;
    } else {
      $v = $v + $dv;
    }
  }
  return $x;
}

function cdf(float $x, int $n): float {
  return $x;
}
