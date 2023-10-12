<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function null_<T>(): ?T {
  return null;
}

function expect_null(null $_): void {}

function test(): void {
  expect_null(null_());
}
