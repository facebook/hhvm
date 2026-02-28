<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_int(int $_): void {}

function test(int $x): void {
  if ($x is nonnull) {
    expect_int($x);
  }
}
