<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function bar(): void {
    $y = 4;
    $x = ($value) ==> $value + $y;
  }
}
