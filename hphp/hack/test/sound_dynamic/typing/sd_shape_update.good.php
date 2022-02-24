<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

type TS = supportdyn<shape('a' => int, ...)>;

<<__SupportDynamicType>>
function expectShape(~TS $_):void { }

<<__SupportDynamicType>>
function getShape():~TS {
  return shape('a' => 3);
}

<<__SupportDynamicType>>
function testit():void {
  $x = getShape();
  $x['a'] = 3;
  expectShape($x);
}
