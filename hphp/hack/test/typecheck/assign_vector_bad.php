<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(Vector<string> $v): Vector<arraykey> {
  $v[0] = 42;
  return $v;
}
