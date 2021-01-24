<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C {}

enum E : int {
  use C;
  A = 1;
}

<<__EntryPoint>>
function main() : void {
  echo E::A . "\n";
}
