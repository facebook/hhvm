<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function varrays(array<int> $a1, varray<string> $a2): void {
  list($a, $b) = $a1;
  hh_show($a);
  hh_show($b);
  list($c, $d) = $a2;
  hh_show($c);
  hh_show($d);
}
