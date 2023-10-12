<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class BadTypeConstraint {
  public function foo<T as arraykey>(T $t) : void {}
  public function bar() : void {
    $this->foo(true);
  }
}
