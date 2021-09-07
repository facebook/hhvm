<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module>>
class A {}

<<__Module('A', 'B')>>
class B {}

<<__Module('C'), __Internal(42)>>
function c(): void {}

<<__Module('D')>>
class D {

  <<__Internal('lol')>>
  public function foobar(): void {}

}
