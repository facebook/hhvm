<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function assign1<T>(inout T $x, T $y):void {
  $x = $y;
}

function assign2<T>(): (function(inout T, T):void) {
  return (inout T $x, T $y) ==> { $x = $y; };
}

function expectArraykey(arraykey $ak):void { }
function inferit():void {
  $v = "a";
  assign1(inout $v, 3);
  expectArraykey($v);
  $assigner = assign2();
  $w = "a";
  $assigner(inout $w, 3);
  expectArraykey($w);
}
