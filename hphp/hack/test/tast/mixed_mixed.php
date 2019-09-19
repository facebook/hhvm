<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expect_mixed(mixed $_): void {}

function test(int $a, string $b): void {
  expect_mixed(
    dict[
      'a' => $a,
      'b' => $b,
    ],
  );
}
