<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f((int, string) $x): void {
  list($a, $b) = $x;
  hh_show($a);
  hh_show($b);
}
