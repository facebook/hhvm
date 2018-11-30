<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait T {
  public function f(): void {}
}
class C {
  use T;

  function g(): void = T::f;
}
