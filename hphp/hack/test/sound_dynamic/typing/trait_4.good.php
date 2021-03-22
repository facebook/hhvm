<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Box<T> {}

class C implements dynamic {
  use T;
}

trait T {
  public function foo(int $x) : int {
    return $x;
  }

  <<__SoundDynamicCallable>>
  public function bar(Box<int> $x) : Box<int> {
    return $x;
  }
}
