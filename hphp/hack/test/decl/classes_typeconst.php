<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  const type T = int;

  public function f(): this::T {
    return 5;
  }

  public function g(): this::T2 {
    return "test";
  }

  const type T2 = string;
}
