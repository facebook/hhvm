<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum Foo : int {
  X = 3;
}

<<__EntryPoint>>
function main(): void {
  $a = Foo::Nonexistent;
}
