<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test1(bool $b, vec<int> $x, vec<string> $y, float $z): void {
  if ($b) {
    $v = $x;
  } else {
    $v = $y;
  }
  $v[0] = $z;
  hh_show($v);
}

function test2(bool $b, Vector<num> $x, Vector<arraykey> $y, int $z): void {
  if ($b) {
    $v = $x;
  } else {
    $v = $y;
  }
  // $v : Vector<num> | Vector<arraykey> and only values that are subtypes of
  // both num and arraykey can be appended to $v.
  $v[0] = $z;
}
