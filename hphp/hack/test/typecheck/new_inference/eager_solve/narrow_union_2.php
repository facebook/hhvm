<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function testit(?Map<string, Map<int,int>> $m):int {
  $m = $m ?? Map {};
  $x = $m["a"];
  $y = $x[1];
  return $y;
}
