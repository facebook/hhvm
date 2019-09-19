<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Box<T> {
  /* HH_FIXME[4336] */
  public function getContents(): T {
  }
}

class VoidBox extends Box<void> {}
