<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function test_is_array_traversable<T>(Traversable<T> $m): ?varray_or_darray<T> {
  if (HH\is_php_array($m)) {
    return $m;
  }
  return null;
}

function test_is_array_keyed_traversable<Tk as arraykey, T>(
  KeyedTraversable<Tk, T> $m,
): ?varray_or_darray<T> {
  if (HH\is_php_array($m)) {
    return $m;
  }
  return null;
}

function test_is_array_no_change<T>(
  varray_or_darray<T> $x,
): ?varray_or_darray<T> {
  if (HH\is_php_array($x)) {
    return $x;
  }
  return null;
}
