<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public function foo(): int { return C::X; }
}

final class C  {

  const X = 3;
}
