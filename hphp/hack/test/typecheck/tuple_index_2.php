<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

/* HH_FIXME[4110] */
function Vec_map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {

}

function test(): vec<int> {
  return Vec_map(
    vec[
      tuple(1, true),
      tuple('two', false)
    ],
    $t ==> {
      $t[0];
      return $t[0];
    },
  );
}
