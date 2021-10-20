<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('upcast_expression')>>

<<__SupportDynamicType>>
class C { }

<<__SupportDynamicType>>
function foo(int $_):bool {
  return false;
}

function expectFun((function(int):bool) $_):void { }

function test1(
  (function(dynamic):int) $f1,
  (function(mixed, dynamic):string) $f2,
  (function((string|dynamic),dynamic...):float) $f3,
  (function():void) $f4,
  (function():C) $f5):void {
  $f1 upcast dynamic;
  $f2 upcast dynamic;
  $f3 upcast dynamic;
  $f4 upcast dynamic;
  $f5 upcast dynamic;
  ($x ==> $x) upcast dynamic;
  ((dynamic $d) ==> $d->foo()) upcast dynamic;
  fun('foo') upcast dynamic;
  expectFun(fun('foo'));
}
