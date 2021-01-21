<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum E : int {
  A = 2;
}

enum F : int {
  use E;
  A = 1;
}

<<__EntryPoint>>
function main() : void {
  echo F::A . "\n";
}
