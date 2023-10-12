<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}

function f(
  vec<A> $va,
): vec<B> {
  return Vec_fb_maybe_map(
    $va,
    $x ==> {
      if ($x is B) {
        return $x;
      }
      return null;
    },
  );
}

function Vec_fb_maybe_map<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): ?Tv2) $value_func,
): vec<Tv2> {
  return vec[];
}
