<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract final class C {}

class D {
  public function foo(): ?C {
    return null;
  }
}
