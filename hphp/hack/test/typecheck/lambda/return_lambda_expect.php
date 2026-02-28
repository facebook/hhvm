<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function bar():void { }
}
function foo():(function():vec<(function(C):void)>) {
  return () ==> vec[$x ==> $x->bar()];
}
