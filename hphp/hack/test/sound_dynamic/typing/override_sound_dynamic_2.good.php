<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  <<__SupportDynamicType>>
  public function foo(int $_):void { }
}

<<__SupportDynamicType>>
class D extends C {
  public function foo(int $_):void { }
}
