<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Ideally Hack should report an error saying that parent::bar returns
// an arraykey, not an int

<<file:__EnableUnstableFeatures('require_class')>>

function expect_int(int $x): void {}

trait T {
  require class D;

  public function foo(): void {
    expect_int(parent::bar());
  }
}

class C {
  public static function bar(): arraykey {
    return "hello";
  }
}

final class D extends C {
  use T;

  public static function bar(): int {
    return 42;
  }
}

<<__EntryPoint>>
function main(): void {
  echo "start\n";
  (new D())->foo();
}
