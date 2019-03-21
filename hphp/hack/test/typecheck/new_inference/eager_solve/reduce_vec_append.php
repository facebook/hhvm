<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function reduce<Tv, Ta>(
  Traversable<Tv> $traversable,
  (function(Ta, Tv): Ta) $accumulator,
  Ta $initial,
): Ta {
  return $initial;
}

function test2(vec<int> $trav): void {
  reduce(
    $trav,
    ($acc_v, $element) ==> {
      $acc_v[] = vec[]; // $acc_v: vec<(#3|vec<nothing>)>
      $acc_v[0][] = $element;
      return $acc_v;
    },
    vec[],
  );
}
