<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('upcast_expression')>>

class C { }
function test1(
  (function(int):int) $f1,
  (function(mixed, dynamic):mixed) $f2,
  (function():vec<mixed>) $f3,
  (function():C) $f4):void {
  $f1 upcast dynamic;
  $f2 upcast dynamic;
  $f3 upcast dynamic;
  $f4 upcast dynamic;
  ((int $x) ==> $x+1) upcast dynamic;
  ($x ==> new C()) upcast dynamic;
}
