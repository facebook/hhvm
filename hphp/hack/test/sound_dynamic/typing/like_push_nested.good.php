<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class MyMap<T1 as supportdyn<mixed>, T2 as supportdyn<mixed>> {}
<<__SupportDynamicType>>
class MyVector<T as supportdyn<mixed>> {}
<<__SupportDynamicType>>
class C { }

function expect1(~MyMap<string,MyVector<C>> $_):void { }
function expect2(~MyMap<~string, MyVector<C>> $_): void {}
function expect3(~MyMap<string,~MyVector<C>> $_): void {}
function expect4(~MyMap<string, MyVector<~C>> $_): void {}
function expect5(~MyMap<string, ~MyVector<~C>> $_): void {}
<<__SupportDynamicType>>
function test1(MyMap<~string,MyVector<C>> $m1, MyMap<string,~MyVector<C>> $m2, MyMap<string,MyVector<~C>> $m3, MyMap<string,~MyVector<~C>> $m4):void {
  expect1($m1);
  expect1($m2);
  expect1($m3);
  expect1($m4);

  expect2($m1);
  expect2($m2);
  expect2($m3);
  expect2($m4);

  expect3($m1);
  expect3($m2);
  expect3($m3);
  expect3($m4);

  expect4($m1);
  expect4($m2);
  expect4($m3);
  expect4($m4);

  expect5($m1);
  expect5($m2);
  expect5($m3);
  expect5($m4);
}
