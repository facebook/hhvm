<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum E: int as int {
  A = 0;
}

enum F: int as int {
  B = 1;
}

enum G: int {
  use E;
}

enum H: int {
  use F;
  use G;
  MY_ADDITIONAL_CONST = 42;
}


<<__EntryPoint>>
function main(): void {
  echo E::A . "\n";
  echo F::B . "\n";
  echo H::A . "\n";
  echo H::B . "\n";
}
