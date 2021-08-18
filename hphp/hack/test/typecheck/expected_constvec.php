<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {
  public function foo():int { return 3; }
}

function test1():ConstVector<(function(C):int)> {
  return ImmVector { $x ==> $x->foo() };
}
function test2():MutableVector<(function(C):int)> {
  return Vector { $x ==> $x->foo() };
}
function test3():ConstMap<int,(function(C):int)> {
  return ImmMap { 1 => $x ==> $x->foo() };
}
function test4():MutableMap<int,(function(C):int)> {
  return Map { 1 => $x ==> $x->foo() };
}
