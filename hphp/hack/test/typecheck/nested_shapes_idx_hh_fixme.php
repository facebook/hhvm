<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type Shape = shape(?'x' => ?shape('y' => int));

function test(Shape $s): void {
  Shapes::idx(
    /* HH_FIXME[4110] */ /* HH_FIXME[4323] */
    Shapes::idx($s, 'x', shape()),
    'y',
    0,
  );
}
