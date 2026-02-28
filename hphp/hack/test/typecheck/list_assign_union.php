<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f(vec<int> $a, (string, bool) $b, bool $x): void {
  $z = $x ? $a : $b;
  list($a, $b) = $z;
  hh_show($a);
  hh_show($b);
}

function g((int, string) $a, Pair<bool, float> $b, bool $x): void {
  $z = $x ? $a : $b;
  list($a, $b) = $z;
  hh_show($a);
  hh_show($b);
}

function h((float, bool) $a, dynamic $b, bool $x): void {
  $z = $x ? $a : $b;
  list($a, $b) = $z;
  hh_show($a);
  hh_show($b);
}
