<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__DynamicallyReferenced>>
class C {
  public function foo():int { return 3; }
}

function getVecLikeInt():vec<~int> {
  return vec[];
}

class D extends C {
  public function bar():string { return "A"; }
  public function likey():int {
    return getVecLikeInt()[0];
  }
}

function expectDyn(dynamic $_):void { }

function testit():void {
  expectDyn(new C());
  expectDyn(new D());
}
