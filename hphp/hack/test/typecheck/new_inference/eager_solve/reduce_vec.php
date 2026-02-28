<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function reduce<Tv, Ta>(
  Traversable<Tv> $traversable,
  (function(Ta, Tv): Ta) $accumulator,
  Ta $initial,
): Ta {
  return $initial;
}

function test(vec<int> $trav): void {
  reduce($trav, ($acc_v, $element) ==> {
    $acc_v[] = $element;
    return $acc_v;
  }, vec[]);
}
