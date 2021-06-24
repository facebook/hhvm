<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A"), __Internal>>
type Ty = int;

// In Signatures /////////////////////////////////

<<__Module("A")>>
class A {
  public function a(Ty $x): void {} // error

  <<__Internal>>
  public function b(Ty $x): void {} // ok
}

// Visibility ////////////////////////////////////

<<__Module("A")>>
function f(): void {
  $x = 1 as Ty; // ok
}

<<__Module("B")>>
function g(Ty $x): void {} // error

function h(Ty $x): void {} // error
