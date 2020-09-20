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

function foo(SubEnum $e) : void {}

<<__EntryPoint>>
function main(): void {
  foo(SuperEnum::A);
}
