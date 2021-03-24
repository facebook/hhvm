<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Foo {
  public function foo(int $x) : int {
    return $x;
  }
}

<<__SoundDynamicCallable>>
class Bar extends Foo {}
