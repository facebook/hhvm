<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function badPair(): int {
  $s = "X";
  $x = Pair<int, int>{$s, "Y"};
  return 0;
}
