<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

/* HH_FIXME[4336] */
function darray_filter<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  ?(function(Tv): bool) $f,
): darray<Tk, Tv> {
}

function is_not_null<T>(T $x): bool {
  return false;
}
class C {}
class D {}
function testit(varray<?C> $vc, varray<?D> $vd): void {
  $func = fun('is_not_null');
  $a = darray_filter($vd, $func);
  $b = darray_filter($vc, $func);
}
