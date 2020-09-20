<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test<T as shape('x' => ?int, ...)>(T $s): int {
  return Shapes::idx($s, 'x', 0);
}
