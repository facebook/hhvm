<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(vec<int> $v): vec<arraykey> {
  $v[0] = 'foo';
  return $v;
}
