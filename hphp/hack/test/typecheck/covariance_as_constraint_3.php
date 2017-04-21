<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface Box<+T> {
  public function get(): T;
}

interface BoxFactory<+T, +Tbox as Box<T>> {
  public function make(): Tbox;
}
