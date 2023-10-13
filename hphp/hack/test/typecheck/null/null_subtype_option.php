<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test1(null $x): ?int {
  return $x;
}

function test2<T>(): ?T {
  return null;
}
