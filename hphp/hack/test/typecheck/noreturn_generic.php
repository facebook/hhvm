<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Box<T> {
  public function getContents(): T {
    throw new Exception();
  }
}

class VoidBox extends Box<void> {}
