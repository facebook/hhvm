<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

class C {
  public static function f(int $d): int {
    C::f(dyn());
    return dyn();
  }
}
