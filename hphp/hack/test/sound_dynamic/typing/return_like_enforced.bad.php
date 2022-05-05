<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// No SDT
function get(): ~vec<int> { return vec[3]; }

<<__SupportDynamicType>>
function get(): ~vec<vec<int>> { return vec[vec[3]]; }

<<__SupportDynamicType>>
function test(~int $x):vec<int> {
  $y = get()[$x];
  return $y;
}
