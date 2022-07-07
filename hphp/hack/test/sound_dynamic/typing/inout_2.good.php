<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function getVec():~vec<int> {
  return vec[3];
}

<<__SupportDynamicType>>
function test(inout int $x):void {
  $x = getVec()[0];
}
