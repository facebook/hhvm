<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum F: int as int {
  A = 0;
}

enum G: int as int {
  B = 1;
}

enum H: int {
  use F;
  use G;
  C = 42;
}

<<__EntryPoint>>
function main(): void {
  echo F::A . "\n";
  echo G::B . "\n";
  echo H::A . "\n";
  echo H::B . "\n";
}
