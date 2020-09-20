<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function union(bool $x, dynamic $d, (int, string) $tup): void {
  $like = $x ? $d : $tup;
  list($a, $b) = $like;
  hh_show($a);
  hh_show($b);
}
