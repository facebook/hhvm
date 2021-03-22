<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectVecInt(vec<int> $vi):void { }

class C {
  <<__SoundDynamicCallable>>
  public function foo(vec<int> $x):int {
    expectVecInt($x);
    return $x[0];
  }
}
