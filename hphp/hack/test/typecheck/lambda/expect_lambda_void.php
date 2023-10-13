<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(int $x): void {}
function expect_void((function(int): void) $f): void {}
function test_it(): void {
  expect_void(($x) ==> foo($x));
}
