<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
function myfilter<Tv as dynamic >(
  ~Traversable<Tv> $traversable,
  ~?supportdyn<(function (Tv): ~bool)> $value_predicate = null,
): ~vec<Tv> {
  return vec[];
}

type TS = supportdyn<shape(
  'score' => float,
  ...
)>;

<<__SupportDynamicType>>
function test(
  ~vec<TS> $v,
  bool $b,
): ~float {
  $e = null;

  $v = myfilter(
    $v, $w ==> $w['score'] > 0,
  );

  if ($b) {
    $e = $v[0];
  }

  if ($e === null) {
    return 0.0;
  }

  return $e['score'];
}
