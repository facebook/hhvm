<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('modules')>>

<<__Module("A"), __Internal>>
function f(): void {}

function main(): void {
  $f = f<>; // error

  $f();
}
