<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f(int ...$x) {
  foreach ($x as $y) {
    var_dump($y);
  }
}

function g() : array<int> {
  return array(1,2,3,4);
}

$h = "g";

f(...$h());
