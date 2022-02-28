<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
class C {
  <<__Deprecated("Do not call me")>>
  public static function foo():void { }

  public function bar(vec<int> $vi):void {
    /* HH_FIXME[4128] */
    C::foo();
  }
}
