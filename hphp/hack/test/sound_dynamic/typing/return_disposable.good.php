<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C implements IDisposable {
  public function __dispose(): void { }
}

<<__SupportDynamicType, __ReturnDisposable>>
function testit(vec<int> $_): C {
  return new C();
}

<<__SupportDynamicType>>
class D {
  <<__ReturnDisposable>>
  public static function foo(vec<int> $_): C {
    return new C();
  }
}
