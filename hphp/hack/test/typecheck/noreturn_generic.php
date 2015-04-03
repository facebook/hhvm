<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Box<T> {
  public function getContents(): T {
    // UNSAFE
  }
}

class VoidBox extends Box<void> {}
