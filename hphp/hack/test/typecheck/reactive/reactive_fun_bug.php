<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__Rx, __AtMostRxAsArgs, __ProvenanceSkipFrame>>
function from_entries_reduce<Tk as arraykey, Tv>(
  <<__MaybeMutable, __OnlyRxIfImpl(\HH\Rx\Traversable::class)>>
  Traversable<(Tk, Tv)> $entries,
  <<__AtMostRxAsFunc>>
  (function(Tv, Tv): Tv) $merge_func,
): dict<Tk, Tv> {
  return dict[];
  }
<<__Rx, __AtMostRxAsArgs, __ProvenanceSkipFrame>>
function concat<Tv>(
  <<__OnlyRxIfImpl(\HH\Rx\Traversable::class)>>
  Traversable<Tv> $first,
  Container<Tv> ...$rest
): vec<Tv> {
  return vec[];
  }
function testit():void {
  from_entries_reduce(vec[], fun('concat'));
}
