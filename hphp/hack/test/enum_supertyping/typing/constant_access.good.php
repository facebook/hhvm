<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum E1: int as int {
  A = 0;
}
enum E2: int as int {
  B = 1;
}
enum F: int as int {
  use E1;
  use E2;
  C = 2;
}

<<__EntryPoint>>
function test(): void {
  print F::C;
  print F::A;
  print F::B;
}
