<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module('here')>>
function foo():void { }

<<__Module('there')>>
class C {
  public function bar():void { }
}

<<__Module('here')>>
type Talias = int;

<<__Module('elsewhere')>>
enum E : int {
  A = 3;
}

<<__Module('here')>>
newtype Topaque = string;
