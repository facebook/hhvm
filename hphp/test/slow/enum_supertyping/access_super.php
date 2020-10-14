<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum E1 : int {
  A = 1;
}

enum F : int includes E1 {}

<<__EntryPoint>>
function main() : void {
  echo F::A . "\n";
}
