<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
function toplevel(vec<int> $vi):int {
  return C::A;
}

<<__SupportDynamicType>>
class C {
  public function foo(vec<int> $vi):int {
    return C::A;
  }
}
