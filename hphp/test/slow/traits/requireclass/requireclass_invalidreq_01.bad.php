<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T2 {}

trait T {
  require class T2;
}

<<__EntryPoint>>
function main(): void {
  echo "done\n";
}
