<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function pair(Pair<int, string> $p): void {
  list($a, $b) = $p;
  hh_show($a);
  hh_show($b);
}
