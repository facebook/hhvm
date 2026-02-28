<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C {
  public function foo(int $_):bool { return false; }
}

function expect1(supportdyn<(function(C,int):bool)> $_):void { }

function testit(~C $c):void {
  $f = meth_caller(C::class, 'foo');
  expect1($f);
}
