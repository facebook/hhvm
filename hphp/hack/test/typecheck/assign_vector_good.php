<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(Vector<arraykey> $v): Vector<arraykey> {
  $v[0] = 42;
  return $v;
}
