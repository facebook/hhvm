<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_int(int $_): void {}

function test1(bool $b, nothing $e): void {
  if ($b) {
    $x = $e;
  } else {
    $x = 42;
  }
  expect_int($x);
}

function expect_Vector_of_int(Vector<int> $_): void {}

function test2(nothing $e): void {
  $v = Vector {$e, 42};
  expect_Vector_of_int($v);
}
