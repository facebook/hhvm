<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A"), __Internal>>
interface A {}

<<__Module("A")>>
interface A2 {}

<<__Module("A")>>
interface A3 {
  <<__Internal>>
  public function f(): void;
  // Ok! But it's not possible to implement this outside the module
}

<<__Module("B")>>
class B implements A {} // Bad

<<__Module("B")>>
class B2 implements A2 {} // Ok
