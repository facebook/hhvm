<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T1 {
  require class C;
}

trait T2 {
  use T1;
}

final class C {
  use T2;
}

<<__EntryPoint>>
function main(): void {
  new C();
  echo "done\n";
}
