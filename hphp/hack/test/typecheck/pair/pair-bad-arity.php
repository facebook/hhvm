<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function badPair(): int {
  $x = Pair<int>{1, 2};
  $y = Pair<int, int, int>{1, 2};
  return 0;
}
