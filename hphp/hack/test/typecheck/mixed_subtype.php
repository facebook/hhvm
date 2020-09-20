<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function A(): nothing {
    throw new Exception();
  }
}

class Bar extends Foo {
  public function A(): mixed {
    throw new Exception();
  }
}
