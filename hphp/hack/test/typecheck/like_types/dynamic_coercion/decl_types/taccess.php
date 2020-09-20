<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

final class C {
  const type T = int;

  public function f(): this::T {
    return dyn(); // error
  }
}
