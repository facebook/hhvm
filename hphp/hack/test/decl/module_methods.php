<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A")>>
class A {
  <<__Internal>>
  public function f(): void {}
}

function main(): void {
  $a = new A();
  $a->f();
}
