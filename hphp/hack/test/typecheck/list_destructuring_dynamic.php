<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function dynamic(dynamic $d): void {
  list($a, $b) = $d;
  hh_show($a);
  hh_show($b);
}
