<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C {
  public function foo():int { return 3; }
}

class D extends C {
  public function bar():string { return "A"; }
}

function expectDyn(dynamic $_):void { }

function testit():void {
  expectDyn(new C());
  expectDyn(new D());
}
