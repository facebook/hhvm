<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public function foo(): int { return C::X; }
}

final class C  {
  use T;
}
