<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum E: string {
  FOO = 'foo';
  BAR = 'bar';
}

function test(E $x): Stringish {
  return $x;
}
