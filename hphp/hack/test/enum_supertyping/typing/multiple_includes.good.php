<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum SubEnumA: int as int {
  A = 0;
}

enum SubEnumB: int as int {
  B = 1;
}

enum SuperEnum: int includes SubEnumA, SubEnumB {
  C = 42;
}

<<__EntryPoint>>
function main(): void {
  echo SubEnumA::A . "\n";
  echo SubEnumB::B . "\n";
  echo SuperEnum::A . "\n";
  echo SuperEnum::B . "\n";
}
