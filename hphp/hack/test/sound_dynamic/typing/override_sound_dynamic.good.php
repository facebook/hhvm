<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('upcast_expression')>>

class C {
  public function foo(int $_):void { }
  public function bar(): dynamic { return ("A" upcast dynamic); }
}
class D extends C {
  public function foo(dynamic $_):void { }
  public function bar(): vec<int> { return vec[]; }
}
