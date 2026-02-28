<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C implements J {
  public function foo():void { }
}

<<__SupportDynamicType>>
interface J { }

function make1():~C {
  throw new Exception("A");
  }

function make2():(~C & J) {
  throw new Exception("A");
  }

<<__SupportDynamicType>>
function testit(vec<int> $_):void {
  $x1 = make1();
  $x2 = make2();
  // Succeeds
  $x1->foo();
  // Fails
  $x2->foo();
}
