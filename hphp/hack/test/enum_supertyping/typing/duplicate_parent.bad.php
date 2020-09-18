<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum SubEnumA: int as int {
  A = 0;
}

enum SuperEnum: int includes SubEnumA {
  A = 42;
}
