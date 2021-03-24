<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Box<T> {
  public function  __construct(public T $x) {}
}

<<__SoundDynamicCallable>>
class C {
  <<__SoundDynamicCallable>>
  public function foo() : Box<int> {
    return new Box(42);
  }
}
