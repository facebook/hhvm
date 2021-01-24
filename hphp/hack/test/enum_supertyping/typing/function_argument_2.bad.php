<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum F: int as int {
  A = 0;
}

enum G: int {
  use F;
  B = 1;
}

function foo(G $e) : G { return $e; }

<<__EntryPoint>>
function main(): void {
  echo foo(F::A);
}
