<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(vec<int> $vs): vec<arraykey> {
  $vs[] = 'foo';
  return $vs;
}
