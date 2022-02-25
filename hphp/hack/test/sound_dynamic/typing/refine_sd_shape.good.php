<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

type TS = supportdyn<shape(?'a' => int, ...)>;

function foo(TS $x):int {
  if (Shapes::idx($x, 'a') !== null) {
    $y = $x['a'];
    return 3;
  }
  else return 0;
}
