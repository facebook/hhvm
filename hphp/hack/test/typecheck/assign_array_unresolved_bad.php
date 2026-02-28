<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(bool $b, Vector<num> $x, Vector<arraykey> $y, string $z): void {
  if ($b) {
    $v = $x;
  } else {
    $v = $y;
  }
  // $v : Vector<num> | Vector<arraykey> and only values that are subtypes of
  // both num and arraykey can be appended to $v.
  $v[0] = $z;
}
