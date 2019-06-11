<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

final class C {
  public function f(): this {
    return dyn(); // error
  }
}
