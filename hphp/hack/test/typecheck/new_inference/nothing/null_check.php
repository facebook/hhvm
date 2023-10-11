<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_nothing(nothing $_): void {}

function test(null $x): void {
  if ($x !== null) {
    expect_nothing($x);
  }
}
