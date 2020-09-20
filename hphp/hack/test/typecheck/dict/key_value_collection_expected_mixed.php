<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expect_mixed(mixed $_): void {}

class C {}

function test(float $x, ?int $y, C $z): void {
  expect_mixed(
    dict[
      $x => 1,
      $y => 2,
      $z => 3,
    ],
  );
}
