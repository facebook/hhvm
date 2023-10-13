<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test((int, int) $t): (int, float) {
  $t[1] = 3.14;
  return $t;
}
