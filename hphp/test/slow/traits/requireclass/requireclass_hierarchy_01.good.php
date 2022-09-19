<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;
}

final class C {
  use T;
}

<<__EntryPoint>>
function main(): void {
  new C();
  echo "done\n";
}
