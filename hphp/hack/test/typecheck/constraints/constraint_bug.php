<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function darray_filter_falsy<Tk as arraykey, Tv as nonnull>(
KeyedTraversable<Tk, ?Tv> $traversable,
): darray<Tk, Tv> {
  return darray[];
}
function testit(mixed $m):void {
  invariant(is_array($m), 'hack');
  $a = darray_filter_falsy($m);
}
