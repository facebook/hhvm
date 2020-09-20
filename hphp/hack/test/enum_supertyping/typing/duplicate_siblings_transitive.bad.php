<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum SubEnumA: int as int {
  A = 0;
}

enum SubEnumC: int as int includes SubEnumA {}

enum SubEnumB: int as int {
  A = 1;
}

enum SuperEnum: int includes SubEnumC, SubEnumB {
}
