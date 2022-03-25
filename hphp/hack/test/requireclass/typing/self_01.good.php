<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public function foo(): void {
    self::bar();
  }
}

class C {
  use T;

  public static function bar(): void {}
}
