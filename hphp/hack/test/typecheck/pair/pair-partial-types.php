<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function badPair(): int {
  $x = Pair<int, _>{1, 2};
  $x = Pair<_, int>{1, 2};
  return 0;
}
