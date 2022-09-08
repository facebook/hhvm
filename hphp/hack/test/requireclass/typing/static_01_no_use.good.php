<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class C;

  public function foo(): void {
    static::bar();
  }
}

final class C {

  public static function bar(): void {}
}
