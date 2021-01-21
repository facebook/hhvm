<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum E : string {
  A = "foo";
}

enum F : int {
  use E;
  B = 1;
}

<<__EntryPoint>>
function main() : void {
  echo F::A . "\n";
}
