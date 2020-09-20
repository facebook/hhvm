<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1 {}
interface I2 {}
interface I3 {}
interface I4 {}
function intersect(mixed $m): void {
  if ($m is (I1, I2) && $m is (I3, I4)) {
    list($a, $b) = $m;
    hh_show($a);
    hh_show($b);
  }
}
