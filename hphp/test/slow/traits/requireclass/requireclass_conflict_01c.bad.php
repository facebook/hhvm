<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

class C { use T1, T2; }

trait T1 { require class C; }
trait T2 { require extends C; }

<<__EntryPoint>>
function main(): void {
  echo "done\n";
}
