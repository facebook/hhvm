<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(
  bool $b,
  Map<string, int> $x,
  Map<string, float> $y,
  num $z,
): void {
  if ($b) {
    $d = $x;
  } else {
    $d = $y;
  }
  $d['foo'] = $z;
}
