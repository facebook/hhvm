<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function expect<T as supportdyn<mixed> >(T $obj, mixed ...$args)[]: ~ExpectObj<T> {
  throw new Exception("A");
}

<<__SupportDynamicType>>
final class ExpectObj<T as supportdyn<mixed> > {
  final public function toBeGreaterThan(num $expected): void where T as num {
  }
}

function getVec():~vec<int> { return vec[]; }
function testit():void {
  $y = getVec()[0];
  // This works
  $z = expect<int>($y);
  $z->toBeGreaterThan(0);
  // This does not, because we infer ~int for the type argument
  // This is due to the call to expect generating ~int <: #0 <: supportdyn<mixed>
  $w = expect($y);
  $w->toBeGreaterThan(0);
}
