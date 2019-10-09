<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public static function foo<reify T>(T $x):void { }
}

function testit():void {
  C::foo<int>(3);
}
