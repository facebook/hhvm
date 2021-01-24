<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum F: int as int {
  A = 0;
}

enum G: int {
  use F;
  B = 1;
}

function foo(F $e) : void {}

<<__EntryPoint>>
function main(): void {
  foo(G::A);
  foo(G::B);
}
