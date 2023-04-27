<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T1 {
  require class C;

  public function foo(): void {
    $this->bar();
  }
}

trait T2 {
  require class C;

  public function bar(): void {
    $this->gee();
  }
}

final class C {
  use T1, T2;

  public function gee(): void {}
}
