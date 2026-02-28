<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
function get(): ~vec<int> { return vec[3]; }

<<__SupportDynamicType>>
function test(~int $x):int {
  $y = get()[$x];
  return $y;
}

<<__SupportDynamicType>>
async function test2(~int $x):Awaitable<int> {
  $y = get()[$x];
  return $y;
}
