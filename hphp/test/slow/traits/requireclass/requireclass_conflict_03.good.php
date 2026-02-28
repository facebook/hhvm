<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

final class C {}
final class D {}

trait T1 { require class C; }
trait T2 { require class D; }

trait T { use T1, T2; }

<<__EntryPoint>>
function main(): void {
  echo "done\n";
}
