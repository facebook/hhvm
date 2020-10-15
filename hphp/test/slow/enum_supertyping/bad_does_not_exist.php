<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum F : int includes E {
  A = 1;
}

<<__EntryPoint>>
function main() : void {
  echo F::A . "\n";
}
