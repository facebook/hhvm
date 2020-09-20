<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function Dict_map<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): dict<Tk, Tv2> {
  return dict[];
}

function test(): void {
  Dict_map(
    dict[
      'a' => tuple(shape('x' => 1), true),
      'b' => tuple(shape('y' => 2), false)
    ],
    $t ==> Shapes::idx($t[0], 'y'),
  );
}
