<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function id<T>(T $x): T {
  return $x;
}

function inferID(int $x): int {
  return id<_>($x);
}
