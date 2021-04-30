<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Box<T> {}

<<__SupportDynamicType>>
class C {
  use T;
}

trait T {
  public function foo(int $x) : int {
    return $x;
  }

  <<__SupportDynamicType>>
  public function bar(Box<int> $x) : Box<int> {
    return $x;
  }
}
