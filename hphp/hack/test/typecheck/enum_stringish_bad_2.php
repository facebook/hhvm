<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum E: int as int {
  FOO = 1;
  BAR = 2;
}

function test(E $x): Stringish {
  return $x;
}
