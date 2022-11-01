<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function darray_filter_falsy<Tk as arraykey, Tv as nonnull>(
KeyedTraversable<Tk, ?Tv> $traversable,
): darray<Tk, Tv> {
  return darray[];
}
function testit(mixed $m):void {
  invariant(HH\is_php_array($m), 'hack');
  // We need to infer nonnull for Tv for this to work, but in most cases
  // it's better to infer the Tv#1 type, so we need the explicit argument.
  $a = darray_filter_falsy<_, nonnull>($m);
}
