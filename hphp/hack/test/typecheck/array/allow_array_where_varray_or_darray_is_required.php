<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_untyped_varray_or_darray(varray_or_darray $x): void {}

function return_untyped_array(): array {
  return varray[];
}

function main(): void {
  expect_untyped_varray_or_darray(return_untyped_array());
}
