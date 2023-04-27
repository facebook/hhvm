<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

function get():~?int {
  return 3;
}

<<__SupportDynamicType>>
function expectDynInt(dynamic $d, ?int $x):void { }

function expectDyn(dynamic $d):void { }

function testit():void {
  expectDyn(get());
  expectDynInt(get(), get());
}
