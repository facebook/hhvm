<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_is_vec<T>(Indexish<int, T> $v): ?vec<T> {
  if (is_vec($v)) {
    return $v;
  }
  return null;
}
