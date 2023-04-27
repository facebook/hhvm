<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

interface I {}

trait T {
  require class I;
}

<<__EntryPoint>>
function main(): void {
  echo "done\n";
}
