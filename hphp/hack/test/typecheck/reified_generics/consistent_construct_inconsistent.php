<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function f(): void {
    new static();
  }
}
