<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// The runtime does not enforce yield
function y(dynamic $i): Generator<int, int, void> {
  yield $i;
}

function test(): void {
  foreach (y("3") as $i) {}
}
