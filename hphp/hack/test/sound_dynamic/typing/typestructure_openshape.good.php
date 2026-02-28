<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C {
  const type TC = supportdyn<shape(...)>;
}

<<__SupportDynamicType>>
function expectShape(supportdyn<shape(...)> $s): void { }

<<__SupportDynamicType>>
function getCN():~classname<C> {
  throw new Exception("A");
}

<<__SupportDynamicType>>
function test():void {
  $cn = getCN();
  $s = type_structure($cn, 'TC')['fields'];
  expectShape($s);
}
