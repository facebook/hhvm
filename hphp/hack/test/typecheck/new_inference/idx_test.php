<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_nullable_int(?int $_): void {}

function test(Vector<int> $is): void {
  $m = Map {'foo' => 42};
  $x = idx($m, 'foo');
  expect_nullable_int($x);
}
