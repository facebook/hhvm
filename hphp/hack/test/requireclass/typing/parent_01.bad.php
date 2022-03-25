<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Ideally Hack should accept this

<<file:__EnableUnstableFeatures('require_class')>>

trait T {
  require class D;

  public function foo(): void {
    parent::bar();
  }
}

class C {
  public static function bar(): void {}
}

class D extends C {
  use T;
}
