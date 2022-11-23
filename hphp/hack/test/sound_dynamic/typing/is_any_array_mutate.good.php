<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function testmutate(vec<int> $v, dynamic $m):void {
  if (!HH\is_any_array($m)) {
    $m = dict[];
  }
  $m[0] = 5;
}
