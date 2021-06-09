<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A")>>
class A {
  <<__Internal>>
  public function __construct() {}
}

function none(): void {
  $a = new A();
}

<<__Module("A")>>
function a(): void {
  $a = new A();
}

<<__Module("B")>>
function b(): void {
  $a = new A();
}
