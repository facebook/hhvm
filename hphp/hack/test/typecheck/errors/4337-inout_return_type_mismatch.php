<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function bar(inout int $x): void {
    $x = "nope";
  }
}
