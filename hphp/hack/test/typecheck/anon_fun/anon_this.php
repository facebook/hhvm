<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const int X = 0;
  public function foo(): void {
    function () {
      $this::X;
    };
  }
}
