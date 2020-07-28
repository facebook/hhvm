<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f(): int {
  $x = array_map<int, int, int>(function($x, $y) { return $x + $y; }, vec[1, 2], vec[2,3]);
  return $x[0];
}
