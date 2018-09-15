<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1 {}
interface I2<T as arraykey> extends I1 {
  public function get(): T;
}

function fun_stuff<T as arraykey>(T $in): void {}

function instanceof_bug(I1 $in): void {
  if ($in instanceof I2) {
    fun_stuff($in->get());
  }
}
