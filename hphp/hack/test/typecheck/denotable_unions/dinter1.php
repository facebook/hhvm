<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {
  public function foo():void;
}
interface J {
  public function bar():void;
}
function testit((I & J) $x):void {
  $x->foo();
  $x->bar();
}
