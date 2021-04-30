<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Box<T> {}

class Foo {
  <<__SupportDynamicType>>
  public function foo(Box<int> $x) : Box<int> {
    return $x;
  }
}

<<__SupportDynamicType>>
class Bar extends Foo {}
