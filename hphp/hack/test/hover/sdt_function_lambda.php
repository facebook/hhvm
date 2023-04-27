<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function first(vec<int> $vi):int {
  $x = 5;
  $g = () ==> {
    return $x;
    //     ^ hover-at-caret
  };
  return $g();
}
