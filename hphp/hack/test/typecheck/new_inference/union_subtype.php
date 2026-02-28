<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(int $x, string $y): vec<arraykey> {
  return vec[$x, $y];
}
