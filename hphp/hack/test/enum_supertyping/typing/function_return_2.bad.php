<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum SubEnum: int as int {
  A = 0;
}

enum SuperEnum: int includes SubEnum {
  B = 1;
}

function foo1() : SuperEnum {
  return SuperEnum::A;
}

function foo2() : SuperEnum {
  return SubEnum::A;
}

function foo3() : SubEnum {
  return SubEnum::A;
}
