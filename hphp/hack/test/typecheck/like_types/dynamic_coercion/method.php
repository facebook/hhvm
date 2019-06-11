<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

class C {
  public function f(int $d): int {
    $this->f(dyn());
    return dyn();
  }
}
