<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_is_vec(mixed $v): ?vec<string> {
  if (is_vec($v)) {
    return $v;
  }
  return null;
}
