<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum E1: int {
  A = 1;
  B = 2;
}

function test1(E1 $e1): XHPChild {
  return $e1;
}

enum E2: int as arraykey {
  X = 42;
  Y = 1337;
}

function test(E2 $e2): XHPChild {
  return $e2;
}
