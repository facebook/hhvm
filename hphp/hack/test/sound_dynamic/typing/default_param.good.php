<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

namespace DefParam;

<<__SupportDynamicType>>
class C {
  public static ~vec<int> $v = vec[3,4];
  public static function foo(int $arg = self::$v[0]):void { }
  }
