<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C implements dynamic {
  public function foo(int ...$x) : int {
    return $x[0];
  }
}
