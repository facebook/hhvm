<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expect<T>(T $_): void {}

function test(int $x, int $y): void {
  function () use ($y) {
    expect<int>($y);
    $x; // should be undefined
  };
}
