<?php

# Compute the natural log of Gamma(x), accurate to 10 decimal places.
#
# This implementation is based on:
#
#   Pike, M.C., I.D. Hill (1966) Algorithm 291: Logarithm of Gamma
#   function [S14].  Communications of the ACM 9(9):684.
function lnGamma($x) {
  assert($x > 0.0);
  $x = (float)$x;

  if ($x < 7.0) {
    $f = 1.0;
    $z = $x;
    while ($z < 7.0) {
      $f *= $z;
      $z += 1.0;
    }
    $x = $z;
    $f = -log($f);
  } else {
    $f = 0.0;
  }

  $z = 1.0 / ($x * $x);

  return $f + ($x-0.5) * log($x) - $x + 0.918938533204673 +
    (((-0.000595238095238 * $z + 0.000793650793651) * $z - 0.002777777777778)
    * $z + 0.083333333333333) / $x;
}

#function gamma($x) {
#  return exp(lnGamma($x));
#}

# Compute the pdf for the t distribution at $t with $n degrees of freedom.
function tPdf($t, $n) {
  $t = (float)$t;
  $n = (float)$n;
  # Transform the calculation to directly use lnGamma() rather than gamma(), in
  # order to avoid numerical instability.
  return exp(lnGamma(($n+1.0)*0.5)
             - log(sqrt($n*pi())) - lnGamma(($n*0.5))
             + log(pow(1.0 + $t*$t/$n, -($n+1.0)*0.5)));
# return gamma(($n+1.0)/2.0)
#        / (sqrt($n*pi())*gamma($n/2.0))
#        * pow(1.0 + $t*$t/$n, -($n+1.0)/2.0);
}

# Compute the cdf for the t distribution, with $n degrees of freedom and
# specified $accuracy.
function tCdf($x, $n, $accuracy=0.001) {
  $neg = ($x < 0.0);
  if ($neg) {
    $x = -$x;
  }
  $i = 0.0;
  $cum = 0.0;
  $tPrev = tPdf(0.0, $n);
  $delta = $accuracy * 0.1;
  for ($i = $delta; $i <= $x; $i += $delta) {
    $t = tPdf($i, $n);
    $cum += (($tPrev + $t) * 0.5) * $delta;
    $tPrev = $t;
  }
  if ($neg) {
    $ret = 0.5 - $cum;
  } else {
    $ret = 0.5 + $cum;
  }
  return $ret;
}

# Compute the inverse cdf for the t distribution, with a confidence of $conf
# and $n degrees of freedom.
#
# $accuracy controls accuracy.  Total error is very close to the same as the
# integration stride, due to the approximated curve interpolating between
# computed points.
function tInv($conf, $n, $accuracy=0.001) {
  assert($conf > 0.5 && $conf < 1.0);
  $i = 0.0;
  $cum = 0.0;
  $lim = $conf - 0.5;
  $tPrev = tPdf(0.0, $n);
  $delta = $accuracy * 0.1;
  for ($i = $delta; $cum < $lim; $i += $delta) {
    $t = tPdf($i, $n);
    $cum += (($tPrev + $t) * 0.5) * $delta;
    $tPrev = $t;
  }
  return $i;
}

# Return an array that contains the pairwise differences between $aArr and
# $bArr.
function sampDiff($aArr, $bArr) {
  assert(count($aArr) == count($bArr));
  $N = count($aArr);
  $dArr = array();
  for ($i = 0; $i < $N; $i++) {
    $dArr[] = $bArr[$i] - $aArr[$i];
  }
  return $dArr;
}

# Return the sample mean for $xArr.
function sampMean($xArr) {
  $N = count($xArr);
  $xMean = 0.0;
  foreach ($xArr as $x) {
    $xMean += $x;
  }
  $xMean /= $N;
  return $xMean;
}

# Return the unbiased squared sample variance for $xArr.
function sampVar2($xArr) {
  $N = count($xArr);
  $xMean = sampMean($xArr);
  $S2 = 0.0;
  for ($i = 0; $i < $N; $i++) {
    $x = $xArr[$i];
    $S2 += $x * $x;
  }
  $S2 -= $N * $xMean * $xMean;
  $S2 /= $N-1.0;
  return $S2;
}

function sampVar($xArr) {
  return sqrt(sampVar2($xArr));
}

# Return the probability associated with Student's t-test for arrays $aArr and
# $bArr.  Also compute the confidence interval for the differences based on
# $conf and place the bounds into $lower and $upper.
function ttest($aArr, $bArr, $welch=true, $sides=2, $paired=true,
               $conf=0.95, &$lower=null, &$upper=null) {
  assert($sides == 2); # 1-sided test not implemented.
  assert($paired); # pooled test not implemented.

  $N = count($aArr);
  $dArr = sampDiff($aArr, $bArr);
  $dMean = sampMean($dArr);
  $dVar = sampVar($dArr);

  if ($welch) {
    $aVar2 = sampVar2($aArr);
    $bVar2 = sampVar2($bArr);
    $tDenom = sqrt(($aVar2+$bVar2) / $N);

    $term = $aVar2 + $bVar2;
    $df = ($N-1.0) * $term*$term / ($aVar2*$aVar2 + $bVar2*$bVar2);
  } else {
    $tDenom = $dVar/sqrt($N);
    $df = $N - 1;
  }

  $inv = tInv($conf, $df);
  $lower = $dMean - ($inv * $tDenom);
  $upper = $dMean + ($inv * $tDenom);

  $t = $dMean/$tDenom;
  $p = 2.0 * (1.0 - tCdf($t, $df));
  return $p;
}

#===============================================================================
# Test code.

function printCdf() {
  $nA = array();
  for ($n = 1; $n <= 30; $n++) {
    $nA[] = $n;
  }
  $nA[] = 40;
  $nA[] = 60;
  $nA[] = 120;
  $nA[] = 1000000;

  printf("%4s", "x");
  foreach ($nA as $n) {
    printf("   df=%2d", $n);
  }
  printf("\n");

  for ($x = -3.5; $x < 3.6; $x += 0.1) {
    printf("%+3.1f", $x);
    foreach ($nA as $n) {
      printf(" %7.3f", tCdf($x, $n, 0.001));
    }
    printf("\n");
  }
}
printCdf();

function printInv() {
  $nA = array();
  for ($n = 1; $n <= 30; $n++) {
    $nA[] = $n;
  }
  $nA[] = 40;
  $nA[] = 60;
  $nA[] = 120;
  $nA[] = 1000000;

  $tA = array(0.6, 0.7, 0.8, 0.9, 0.95, 0.975, 0.99, 0.995);

  printf("%7s", "df");
  foreach ($tA as $t) {
    printf(" t=%4.3f", $t);
  }
  printf("\n");

  foreach ($nA as $n) {
    printf("%7d", $n);
    foreach ($tA as $t) {
      printf(" %7.3f", tInv($t, $n, 0.001));
    }
    printf("\n");
  }
}
printInv();

function printTtest() {
  $before = array(25, 25, 27, 44, 30, 67, 53, 53, 52, 60, 28);
  printf("before:");
  foreach ($before as $elm) { printf(" %d", $elm); }
  printf("\n");

  $after = array(27, 29, 37, 56, 46, 82, 57, 80, 61, 59, 43);
  printf("after:");
  foreach ($after as $elm) { printf(" %d", $elm); }
  printf("\n");

  $dArr = sampDiff($before, $after);
  printf("differences:");
  foreach ($dArr as $elm) { printf(" %d", $elm); }
  printf("\n");

  $dMean = sampMean($dArr);
  $dVar = sampVar($dArr);
  printf("mean: %.5f, var: %.5f\n", sampMean($dArr), sampVar($dArr));
  $p = ttest($before, $after, true, 2, true, 0.95, $lower, $upper);
  printf("ttest(before, after, welch=true, sides=2, paired=true)"
         . " --> %.5f (%.5f .. %.5f)\n",
         $p, $lower, $upper);
  $p = ttest($before, $after, false, 2, true, 0.95, $lower, $upper);
  printf("ttest(before, after, welch=false, sides=2, paired=true)"
         . " --> %.5f (%.5f .. %.5f)\n",
         $p, $lower, $upper);
}
printTtest();
