<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function darray_filter<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  ?(function(Tv): bool) $f,
): darray<Tk, Tv> {
  return darray[];
}

function is_not_null<T>(T $x): bool {
  return false;
}
class C {}
class D {}
function testit(varray<?C> $vc, varray<?D> $vd): void {
  $func = is_not_null<>;
  $a = darray_filter($vd, $func);
  $b = darray_filter($vc, $func);
}
