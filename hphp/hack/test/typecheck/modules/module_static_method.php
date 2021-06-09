<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A")>>
class A {
  <<__Internal>>
  public static function f(): void {}
}

function none(): void {
  A::f();
}

<<__Module("A")>>
function a(): void {
  A::f();
}

<<__Module("B")>>
function b(): void {
  A::f();
}
