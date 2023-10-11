<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expect<T>(T $_): void {}

function test(?int $x): void {
  try {
    if ($x === null) {
      return;
    }
  } catch (Exception $_) {
    expect<int>($x);
  }
}
