<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

type Shape = shape(?'x' => ?shape('y' => int));

function test(Shape $s): void {
  Shapes::idx(
    Shapes::idx($s, 'x', shape()),
    'y',
    0,
  );
}
