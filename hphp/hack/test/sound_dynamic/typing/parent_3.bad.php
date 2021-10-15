<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Box<<<__RequireDynamic>> T> {}

class Foo {
  public function foo(Box<int> $x) : Box<int> {
    return $x;
  }
}

<<__SupportDynamicType>>
class Bar extends Foo {}
