<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public function foo(): void {}
  public function bar(): void {}
}

class C {
  use T;
}

class D extends C {
  <<__Override>>
  public function foo(): void {}
}
