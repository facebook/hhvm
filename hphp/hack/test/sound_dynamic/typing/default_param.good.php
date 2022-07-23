<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

namespace DefParam;

function getVecInt()[]:vec<int> {
  return vec[5,6];
}

<<__SupportDynamicType>>
class C {
  public static ~vec<int> $lvi = vec[3,4];
  // Enforced, so we permit default expression to have type ~int but then the parameter should have type int in the body
  public static function foo(int $arg = self::$lvi[0]):void { hh_expect<int>($arg); }
  // Not enforced, but we permit default expression to have type ~vec<int> and this is the type that it will have in the body
  public static function bar(vec<int> $arg = self::$lvi):void { hh_expect_equivalent<~vec<int>>($arg); }
  // Not enforced, but we preserve the type vec<int> of the default expression through to the body
  public static function boo(vec<int> $arg = getVecInt()):void { expectVecInt($arg); }
  }

<<__SupportDynamicType>>
function expectVecInt(vec<int> $_):void { }
