<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A")>>
class A {
  <<__Internal>>
  public static int $x = 0;
}

function none(): void {
  A::$x = 1;
}

<<__Module("A")>>
function a(): void {
  A::$x = 1;
}

<<__Module("B")>>
function b(): void {
  A::$x = 1;
}
