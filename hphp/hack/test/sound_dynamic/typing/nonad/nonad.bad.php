<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__NoAutoDynamic>>
class C {}

<<__NoAutoDynamic>>
function foo():void { }

class D {
  <<__NoAutoDynamic>>
  public function bar():void { }
}
