<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function getVecLikeInt():vec<~int> {
  return vec[];
}
<<__DynamicallyReferenced>>
class C {
  public function foo(vec<int> $x): vec<num> {
    return vec[$x[0]];
  }
  public function bar():int {
    return getVecLikeInt()[0];
  }
}

function expectDyn(dynamic $_): void {}

function testit(dynamic $d):void {
  expectDyn(new C());
  $c = new C();
  $c->foo($d);
}
