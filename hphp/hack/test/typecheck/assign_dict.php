<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(dict<string, int> $v): dict<arraykey, num> {
  $v[42] = 3.14;
  return $v;
}
