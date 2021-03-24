<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Box<T> {}

class Foo {
  public function foo(Box<int> $x) : Box<int> {
    return $x;
  }
}

<<__SoundDynamicCallable>>
class Bar extends Foo {}
