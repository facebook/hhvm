<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function preserve_null<T as ?int>(T $x): T {
  return $x;
}

function test1(int $x): int {
  return preserve_null($x); // Success
}

function test2(?int $x): ?int {
  return preserve_null($x); // Fails
}
