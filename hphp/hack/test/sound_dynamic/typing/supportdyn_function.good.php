<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__SupportDynamicType>>
function foo(int $x):bool {
  return $x === 3;
}

function expectSDfun(supportdyn<(function(int):bool)> $f):void { }
function expectFun((function(int):bool) $f):void { }
function expectLikeSDfun(~supportdyn<(function(int):bool)> $f):void { }
function test1():void {
  expectSDfun(foo<>);
  expectLikeSDfun(foo<>);
  expectFun(foo<>);
}
