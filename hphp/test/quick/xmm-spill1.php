<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function foo($val, $a, $b, $c, $d, $e, $f, $g, $h, $i, $j, $k, $l, $m, $n, $o,
             $p, $q) {
  $a = $b;
  $b = $c;
  $c = $d;
  $d = $e;
  $e = $f;
  $f = $g;
  $g = $h;
  $h = $i;
  $i = $j;
  $j = $k;
  $k = $l;
  $l = $m;
  $m = $n;
  $n = $o;
  $o = $p;
  $p = $p;
  $q = $val;
  $sum = $a + $b + $c + $d + $e + $f + $g + $h + $i + $j + $k + $l + $m + $n +
         $o + $p + $q;
  $prod = $a * $b * $c * $d * $e * $f * $g * $h * $i * $j * $k * $l * $m * $n *
         $o * $p + $q;
  $res = $prod + $sum;
  return $res;
}

var_dump(foo(500.5, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.1,
             12.2, 13.3, 14.4, 15.5, 16.6, 17.7));
