<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class C {
  public function f(~this $t): this {
    return $t; // error
  }
}
