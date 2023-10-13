<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__SupportDynamicType>>
function expect_int(int $i): void {}

function f<T as ~int>(T $t): void {
  expect_int($t);
}

abstract class X {
  abstract const type T as ~int;

  public function f(this::T $t): void {
    expect_int($t);
  }
}
