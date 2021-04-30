<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Box<T> {
  public function  __construct(public T $x) {}
}

class C {
  <<__SupportDynamicType>>
  public function foo() : Box<int> {
    return new Box(42);
  }
}
