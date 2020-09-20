<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {}

function myflip<Tk, Tv as arraykey>(
  KeyedTraversable<Tk, Tv> $traversable,
): dict<Tv, Tk> {
  $result = dict[];
  foreach ($traversable as $key => $value) {
    $result[$value] = $key;
  }
  return $result;
}
function mymap<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): dict<Tk, Tv2> {
  $result = dict[];
  foreach ($traversable as $key => $value) {
    $result[$key] = $value_func($value);
  }
  return $result;
}
function test_it(vec<C> $p, (function(C): C) $f): void {
  $f = myflip(mymap($p, ($p) ==> $p));
}
