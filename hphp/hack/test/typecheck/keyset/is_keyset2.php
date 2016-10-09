<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_is_keyset<T as arraykey>(Traversable<T> $m): ?keyset<T> {
  if (is_keyset($m)) {
    return $m;
  }
  return null;
}
