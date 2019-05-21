<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function expectLikeNum(~num $y):void { }
function expectLikeInt(~int $z):void { }
function foo<T as ~int>(T $x):void {
  expectLikeNum($x);
  expectLikeInt($x);
}
function bar<T as int>(T $x):void {
  expectLikeNum($x);
  expectLikeInt($x);
}
