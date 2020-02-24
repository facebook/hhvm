<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__Rx, __AtMostRxAsArgs>>
function from_entries_reduce<Tv>(
  <<__AtMostRxAsFunc>>
  (function(Tv, Tv): Tv) $merge_func,
): vec<Tv> {
  return vec[];
}

<<__Rx, __AtMostRxAsArgs>>
function concat(
  <<__OnlyRxIfImpl(     // status when args achecked CONTRAvariantly:
  // Traversable::class
  \HH\Rx\Traversable::class
  )>>
  Traversable<mixed> $first,
  Container<mixed> $rest
): vec<nothing> {
  return vec[];
}

function testit():void {
  $x = from_entries_reduce(fun('concat'));
}
