<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function f_tuple((int, int) $t) : void {}

class C implements dynamic {
  public function foo((int, int) ...$x) : void {
    f_tuple($x[0]);
  }
}
